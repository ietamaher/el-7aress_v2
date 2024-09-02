#include "camerawidget.h"
#include <iostream>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>

int CameraWidget::frame_number = 0;
constexpr gchar CameraWidget::pgie_classes_str[4][32];

CameraWidget::CameraWidget(QWidget *parent) : QOpenGLWidget(parent)    {
    pipeline = NULL, source = NULL, sink = NULL, pgie = NULL, tracker = NULL, streammux = NULL, nvvidconvsrc = NULL, vidconv = NULL,
        nvosd = NULL, transform = NULL, caps1 = NULL, caps2 = NULL, bus = NULL, msg = NULL,  capsfilter1 = NULL,
        capsfilter2 = NULL, decoder = NULL, bus_watch_id = 0, enableDetection = false, enableTracking = false,
        currentBurstMode= "" , currentTrackingState= false, currentDetectionState= false , currentStabState= "", currentSpeed = 0.0, currentAzimuth = 0.0,
        currentincr_ = 0, currentInput1 = false , currentincr_1 = 0,
        setAttribute(Qt::WA_NativeWindow);
    qRegisterMetaType<ObjectInfo>("ObjectInfo");  // Register ObjectInfo for use in signals.
    qRegisterMetaType<QList<ObjectInfo>>("QList<ObjectInfo>");

    setAttribute(Qt::WA_NativeWindow, true);
    setStyleSheet("background-color: green");
    connect(this, &CameraWidget::frameReady, this, &CameraWidget::displayFrame, Qt::AutoConnection);
}

CameraWidget::~CameraWidget() {
}

void CameraWidget::setSettingParameters(const QString& burstMode, bool trackingState, bool detectionState, bool stabState, double speed){
    currentBurstMode = burstMode;
    currentTrackingState = trackingState;
    currentDetectionState = detectionState;
    currentStabState = stabState;
    currentSpeed = speed/100,0;
}

void CameraWidget::setMotorParameters(int incr_, double azimuth){
    currentincr_ = incr_;
    currentAzimuth = azimuth;
}

void CameraWidget::setPlcParameters(int incr_, bool input1){
    currentincr_1 = incr_;
    currentInput1 = input1;
}

void CameraWidget::startCamera() {
    gst_init(NULL, NULL);

    cudaGetDevice(&current_device);
    struct cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, current_device);

    // Create the elements
    pipeline = gst_pipeline_new("camera-pipeline");
    source = gst_element_factory_make("v4l2src", "source");
    g_object_set(source, "device", "/dev/video0", NULL);

    capsfilter1 = gst_element_factory_make ("capsfilter", "caps-filter1");
    caps1 = gst_caps_new_simple("image/jpeg",
                                "format", G_TYPE_STRING, "MJPG",
                                "width", G_TYPE_INT, 1280,
                                "height", G_TYPE_INT, 720,
                                "framerate", GST_TYPE_FRACTION, 30, 1,
                                NULL);
    g_object_set (G_OBJECT (capsfilter1), "caps", caps1, NULL);
    gst_caps_unref (caps1);
    decoder = gst_element_factory_make ("jpegdec", "jpegdec-decoder");

    nvvidconvsrc = gst_element_factory_make("nvvideoconvert", "convertor_src");
    capsfilter2 = gst_element_factory_make ("capsfilter", "caps-filter2");
    caps2 = gst_caps_from_string ("video/x-raw(memory:NVMM), format=(string)NV12");
    g_object_set (G_OBJECT (capsfilter2), "caps", caps2, NULL);
    gst_caps_unref (caps2);
    streammux = gst_element_factory_make ("nvstreammux", "stream-muxer");
    pgie = gst_element_factory_make ("nvinfer", "primary-nvinference-engine");
    g_object_set (G_OBJECT (pgie), "config-file-path", "/home/jetson/Desktop/deep_qt/pgie_config.txt", NULL);
    tracker = gst_element_factory_make ("nvtracker", "nvtracker_");
    g_object_set (G_OBJECT (tracker), "config-file-path", "dstest1_pgie_config.txt", NULL);
    g_object_set(G_OBJECT(tracker), "ll-lib-file", "/opt/nvidia/deepstream/deepstream-6.0/lib/libnvds_nvmultiobjecttracker.so", NULL);
    g_object_set(G_OBJECT(tracker), "ll-config-file", "/opt/nvidia/deepstream/deepstream/samples/configs/deepstream-app/config_tracker_NvDCF_perf.yml", NULL);
    g_object_set(G_OBJECT(tracker), "enable-batch-process", TRUE, NULL);
    g_object_set(G_OBJECT(tracker), "enable-past-frame", TRUE, NULL);
    g_object_set(G_OBJECT(tracker), "display-tracking-id", TRUE, NULL);
    g_object_set(G_OBJECT(tracker), "tracker-width", 640, NULL);
    g_object_set(G_OBJECT(tracker), "tracker-height", 384, NULL);
    vidconv = gst_element_factory_make ("nvvideoconvert", "nvvideo-converter");
    nvosd = gst_element_factory_make ("nvdsosd", "nv-onscreendisplay");
    if (USE_OPENCV ==1){
        sink = gst_element_factory_make ("appsink", "nvvideo-renderer");
        if (!source || !capsfilter1 || !decoder || !nvvidconvsrc  || !capsfilter2  || !streammux  || !pgie  || !tracker || !vidconv  || !nvosd  || !sink ||  !pipeline  ) {
            qWarning("One element could not be created. Exiting.");
            return;
        }
    }else{
        transform = gst_element_factory_make("nvegltransform", "nvegl-transform");
        sink = gst_element_factory_make ("nveglglessink", "nvvideo-renderer");
        if (  !sink || !transform  ) {
            qWarning("sink or  transform element could not be created. Exiting.");
            return;
        }
    }

    g_object_set (G_OBJECT (streammux), "batch-size", 1, NULL);
    g_object_set (G_OBJECT (streammux), "width", MUXER_OUTPUT_WIDTH, "height", MUXER_OUTPUT_HEIGHT, "batched-push-timeout", MUXER_BATCH_TIMEOUT_USEC, NULL);
    g_object_set(G_OBJECT(streammux), "num-surfaces-per-frame", 1, NULL);
    g_object_set(G_OBJECT(streammux), "live-source", TRUE, NULL);

    g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);
    if (USE_OPENCV ==1){
        qDebug() << "Using OpenCV setting emit-signals ...\n";
        g_object_set(sink, "emit-signals", TRUE, NULL);
        g_signal_connect(sink, "new-sample", G_CALLBACK(CameraWidget::on_new_sample), this);
    }

    /* we add a message handler */
    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    loop = g_main_loop_new (NULL, FALSE);
    bus_watch_id = gst_bus_add_watch (bus, CameraWidget::busCall, loop);
    gst_object_unref (bus);

    if (USE_OPENCV ==1){
        gst_bin_add_many (GST_BIN (pipeline), source,  capsfilter1, decoder, nvvidconvsrc,  capsfilter2,  streammux,  pgie, tracker, vidconv, sink, NULL);

    }else{
        gst_bin_add_many (GST_BIN (pipeline), source,  capsfilter1, decoder, nvvidconvsrc,  capsfilter2,  streammux,  pgie, tracker, vidconv, nvosd, transform, sink, NULL);

    }

    GstPad *sinkpad = gst_element_get_request_pad(streammux, "sink_0");
    GstPad *srcpad = gst_element_get_static_pad(capsfilter2, "src");
    if (gst_pad_link(srcpad, sinkpad) != GST_PAD_LINK_OK) {
        g_printerr("Failed to link capsfilter2 to streammux. Exiting.\n");
        return ;
    }
    gst_object_unref(srcpad);
    gst_object_unref(sinkpad);


    if (!gst_element_link_many(source, capsfilter1, decoder,  nvvidconvsrc , capsfilter2, NULL)) {
        qWarning("Elements could not be linked. Exiting.");
        return;
    }
    if (USE_OPENCV ==1){
        if (!gst_element_link_many(streammux, pgie, tracker, vidconv,  sink, NULL)) {
            qWarning("OPCV =1Elements streammux pgie, vvidconv, nvosd, sink could not be linked. Exiting.");
            return;
        }
    }else{
        if (!gst_element_link_many(streammux, pgie, tracker, vidconv,   nvosd, transform, sink, NULL)) {
            qWarning("Elements streammux pgie, vvidconv, nvosd, sink could not be linked. Exiting.");
            return;
        }
    }

    GstPad *osd_sink_pad = gst_element_get_static_pad (nvosd, "sink");
    if (!osd_sink_pad)
        g_print ("Unable to get sink pad\n");
    else
        gst_pad_add_probe (osd_sink_pad, GST_PAD_PROBE_TYPE_BUFFER,
                          osd_sink_pad_buffer_probe, this, NULL);
    gst_object_unref (osd_sink_pad);

    GstPad *tracker_srcPad = gst_element_get_static_pad(tracker, "src");

    if (tracker_srcPad) {
        //gst_pad_add_probe(tracker_srcPad, GST_PAD_PROBE_TYPE_BUFFER, CameraWidget::metadata_probe, NULL, NULL);
        gst_pad_add_probe(tracker_srcPad, GST_PAD_PROBE_TYPE_BUFFER, CameraWidget::metadata_probe, this, NULL);
        gst_object_unref(tracker_srcPad);
    } else {
        g_print("Could not get src pad from nvinfer element\n");
    }
    //gst_object_unref(nvinfer_element);


    // Set the window ID where to render the video
    if (USE_OPENCV ==0){
        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(sink), (guintptr)this->winId());
    }

    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");

    // Start playing
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    /* Wait till pipeline encounters an error or EOS */
    g_print ("Running...\n");

}

GstFlowReturn CameraWidget::on_new_sample(GstAppSink *sink, gpointer data) {
    CameraWidget* instance = static_cast<CameraWidget*>(data);  // Cast user data back to CameraWidget instance
    GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
    qDebug() << "Emitting frameReady from instance:" << instance;

    GstCaps* caps = gst_sample_get_caps (sample);
    if (!caps) {
        qDebug() << "could not get snapshot format\n";
        exit (-1);
    }
    gint width, height;
    GstStructure* s = gst_caps_get_structure (caps, 0);
    const gchar* format = gst_structure_get_string(s, "format");
    qDebug() << "format:" << format  ;

    int res = gst_structure_get_int (s, "width", &width)
              | gst_structure_get_int (s, "height", &height);
    if (!res) {
        qDebug() << "could not get snapshot dimension\n";
        exit (-1);
    }
    qDebug() << "width:" << width << " height:"<< height;
    if (sample) {
        GstBuffer* buffer = gst_sample_get_buffer(sample);
        GstMapInfo map;
        if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
            //qDebug() << "Buffer mapped successfully. Data size:" << map.size;
            // Check if the size is expected
            QImage image(map.data, width, height, QImage::Format_RGB888);
            if (!image.isNull()) {
                int width = 1280;   // You need to set these based on your actual video dimensions
                int height = 720; // You need to set these based on your actual video dimensions

                cv::Mat result;
                if (format =="NV12"){
                    // Assuming 'map.data' points to the NV12 data, and 'width' and 'height' specify the dimensions of the Y plane.
                    cv::Mat yPlane(height, width, CV_8UC1, map.data);  // Y plane is single-channel
                    cv::Mat uvPlane(height / 2, width / 2, CV_8UC2, map.data + width * height);  // UV plane is half the height and width, 2 channels

                    cv::cvtColorTwoPlane(yPlane, uvPlane, result, cv::COLOR_YUV2RGB_NV12);  // Convert NV12 to RGB

                }else{
                    // Convert YUV to RGB using OpenCV
                    cv::Mat temp_mat(cv::Size(width, height), CV_8UC2, map.data);
                    cv::cvtColor(temp_mat, result, cv::COLOR_YUV2RGB_YUY2);
                }
                // Convert format: NV12(CV_8UC1) to RGB using OpenCV


                if (!result.empty()) {
                    // qDebug() << "Emitting valid QImage";

                    QImage rgbImage(result.data, result.cols, result.rows, QImage::Format_RGB888);
                    rgbImage = rgbImage.copy();

                    emit instance->frameReady(rgbImage);
                }
                // Emit the signal with the frame
            }else {
                qDebug() << "Failed to create QImage";
            }
            gst_buffer_unmap(buffer, &map);
        }
        gst_sample_unref(sample);
    }
    return GST_FLOW_OK;
}
// Property probe callback function to toggle the inclusion of "pgie"
GstPadProbeReturn CameraWidget::interval_property_probe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {

    g_object_set(G_OBJECT(pgie), "interval", G_MAXINT, NULL);

    g_print ("entering interval_property_probe... \n");

    return GST_PAD_PROBE_OK;
}

GstPadProbeReturn CameraWidget::osd_sink_pad_buffer_probe (GstPad * pad, GstPadProbeInfo * info, gpointer u_data)
{
    CameraWidget* instance = static_cast<CameraWidget*>(u_data);
    GstBuffer *buf = (GstBuffer *) info->data;
    int num_rects = 0;
    NvDsObjectMeta *obj_meta = NULL;
    int vehicle_count = 0;
    int person_count = 0;
    NvDsMetaList * l_frame = NULL;
    NvDsMetaList * l_obj = NULL;
    NvDsDisplayMeta *display_meta = NULL;
    int remainingSpace = MAX_DISPLAY_LEN;

    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (buf);

    for (l_frame = batch_meta->frame_meta_list; l_frame != NULL;
         l_frame = l_frame->next) {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) (l_frame->data);
        int offset = 0;
        for (l_obj = frame_meta->obj_meta_list; l_obj != NULL;
             l_obj = l_obj->next) {
            obj_meta = (NvDsObjectMeta *) (l_obj->data);
            if (obj_meta->class_id == PGIE_CLASS_ID_VEHICLE) {
                vehicle_count++;
                num_rects++;
            }
            if (obj_meta->class_id == PGIE_CLASS_ID_PERSON) {
                person_count++;
                num_rects++;
            }
        }
        display_meta = nvds_acquire_display_meta_from_pool(batch_meta);
        //NvOSD_TextParams *txt_params  = &display_meta->text_params[0];
        display_meta->num_labels = 11;
        display_meta->num_lines += 2; // 2 + 1 + 36
        //txt_params->display_text = static_cast<char*>(g_malloc0(MAX_DISPLAY_LEN));

        NvOSD_TextParams *txt_params1 = &display_meta->text_params[0];
        txt_params1->display_text = static_cast<char*>(g_malloc0(MAX_DISPLAY_LEN));
        snprintf(txt_params1->display_text, MAX_DISPLAY_LEN, "AZDKD1 = %d", instance-> currentincr_);
        txt_params1->x_offset = 10;
        txt_params1->y_offset = 10;

        NvOSD_TextParams *txt_params2 = &display_meta->text_params[1];
        txt_params2->display_text = static_cast<char*>(g_malloc0(MAX_DISPLAY_LEN));
        snprintf(txt_params2->display_text, MAX_DISPLAY_LEN, "PLC = %d", instance-> currentincr_1);
        txt_params2->x_offset = 10;
        txt_params2->y_offset = 40;

        NvOSD_TextParams *txt_params3 = &display_meta->text_params[2];
        txt_params3->display_text = static_cast<char*>(g_malloc0(MAX_DISPLAY_LEN));
        // Convert QString to char* for use with snprintf
        QByteArray byteArray = instance->currentBurstMode.toUtf8();
        const char *cString = byteArray.constData();
        snprintf(txt_params3->display_text, MAX_DISPLAY_LEN, "%s ", cString);
        txt_params3->x_offset = 10;
        txt_params3->y_offset = 680;

        NvOSD_TextParams *txt_params4 = &display_meta->text_params[3];
        txt_params4->display_text = static_cast<char*>(g_malloc0(MAX_DISPLAY_LEN));
        snprintf(txt_params4->display_text, MAX_DISPLAY_LEN, "SPEED:%d", static_cast<int>(instance->currentSpeed));
        txt_params4->x_offset = 1130;
        txt_params4->y_offset = 210;

        NvOSD_TextParams *txt_params5 = &display_meta->text_params[4];
        txt_params5->display_text = static_cast<char*>(g_malloc0(MAX_DISPLAY_LEN));
        snprintf(txt_params5->display_text, MAX_DISPLAY_LEN, "TRACK:%s ", instance->currentTrackingState ? "ON" : "OFF");
        txt_params5->x_offset = 450;
        txt_params5->y_offset = 10;

        NvOSD_TextParams *txt_params6 = &display_meta->text_params[5];
        txt_params6->display_text = static_cast<char*>(g_malloc0(MAX_DISPLAY_LEN));
        snprintf(txt_params6->display_text, MAX_DISPLAY_LEN, "STAB:%s", instance->currentStabState ? "ON" : "OFF");
        txt_params6->x_offset = 600;
        txt_params6->y_offset = 10;

        NvOSD_TextParams *txt_params7 = &display_meta->text_params[6];
        txt_params7->display_text = static_cast<char*>(g_malloc0(MAX_DISPLAY_LEN));
        snprintf(txt_params7->display_text, MAX_DISPLAY_LEN, "CAM:%s", instance->currentDetectionState ? "NIGHT" : "DAY");
        txt_params7->x_offset = 750;
        txt_params7->y_offset = 10;

        NvOSD_TextParams *txt_params8 = &display_meta->text_params[7];
        txt_params8->display_text = static_cast<char*>(g_malloc0(MAX_DISPLAY_LEN));
        snprintf(txt_params8->display_text, MAX_DISPLAY_LEN, "AZ:%0.1f°",  instance->currentAzimuth);
        txt_params8->x_offset = 1130;
        txt_params8->y_offset = 120;

        NvOSD_TextParams *txt_params9 = &display_meta->text_params[8];
        txt_params9->display_text = static_cast<char*>(g_malloc0(MAX_DISPLAY_LEN));
        snprintf(txt_params9->display_text, MAX_DISPLAY_LEN, "EL:%0.1f°",  12.5);
        txt_params9->x_offset = 1130;
        txt_params9->y_offset = 150;

        NvOSD_TextParams *txt_params10 = &display_meta->text_params[9];
        txt_params10->display_text = static_cast<char*>(g_malloc0(MAX_DISPLAY_LEN));
        snprintf(txt_params10->display_text, MAX_DISPLAY_LEN, "FOV:%d°",  43);
        txt_params10->x_offset = 1130;
        txt_params10->y_offset = 180;

        NvOSD_TextParams *txt_params11 = &display_meta->text_params[10];
        txt_params11->display_text = static_cast<char*>(g_malloc0(MAX_DISPLAY_LEN));
        snprintf(txt_params11->display_text, MAX_DISPLAY_LEN, "LRF:%dm",  265);
        txt_params11->x_offset = 10;
        txt_params11->y_offset = 650;


        instance->setCommonTextParams(&display_meta->text_params[0]);
        instance->setCommonTextParams(&display_meta->text_params[1]);
        instance->setCommonTextParams(&display_meta->text_params[2]);
        instance->setCommonTextParams(&display_meta->text_params[3]);
        instance->setCommonTextParams(&display_meta->text_params[4]);
        instance->setCommonTextParams(&display_meta->text_params[5]);
        instance->setCommonTextParams(&display_meta->text_params[6]);
        instance->setCommonTextParams(&display_meta->text_params[7]);
        instance->setCommonTextParams(&display_meta->text_params[8]);
        instance->setCommonTextParams(&display_meta->text_params[9]);
        instance->setCommonTextParams(&display_meta->text_params[10]);

        // Azimuth in degrees, convert to radians for trigonometric functions
        double azimuthRadians = (instance->currentAzimuth * M_PI) / 180.0;
        int centerX_AZ = 1180;
        int centerY_AZ = 60;
        int radius = 50;
        // Calculate the endpoint of the line based on the azimuth angle
        int endX = centerX_AZ + (radius - 3) * sin(azimuthRadians);
        int endY = centerY_AZ - (radius - 3) * cos(azimuthRadians);

        NvOSD_LineParams *azimuthLine = &display_meta->line_params[display_meta->num_lines++];
        azimuthLine->x1 = centerX_AZ;
        azimuthLine->y1 = centerY_AZ;
        azimuthLine->x2 = endX;
        azimuthLine->y2 = endY;
        azimuthLine->line_width = 2; // Set the line width
        azimuthLine->line_color = (NvOSD_ColorParams){1, 0, 0, 0.8};

        int numSegments = 36; // Number of segments to approximate the circle

        for (int i = 0; i < numSegments; ++i) {
            double angle1 = (double)i * 2 * M_PI / numSegments;
            double angle2 = (double)(i + 1) * 2 * M_PI / numSegments;

            int x1 = centerX_AZ + radius * cos(angle1);
            int y1 = centerY_AZ + radius * sin(angle1);
            int x2 = centerX_AZ + radius * cos(angle2);
            int y2 = centerY_AZ + radius * sin(angle2);

            // Draw line segment from (x1, y1) to (x2, y2)
            NvOSD_LineParams *line_params = &display_meta->line_params[display_meta->num_lines++];
            line_params->x1 = x1;
            line_params->y1 = y1;
            line_params->x2 = x2;
            line_params->y2 = y2;
            line_params->line_width = 2; // Adjust as needed
            line_params->line_color = (NvOSD_ColorParams){1, 0, 0, 0.8};
        }



        //setCommonTextParams(&display_meta->text_params[1]);
        /* Font , font-color and font-size */
        // Assuming you know the center (centerX, centerY) and the size of the crosshair (length)
        int centerX = MUXER_OUTPUT_WIDTH / 2;
        int centerY = MUXER_OUTPUT_HEIGHT / 2;
        int length = 40; // Length of each line of the crosshair

        // Horizontal line
        NvOSD_LineParams *line_params_h = &display_meta->line_params[display_meta->num_lines++];
        line_params_h->x1 = centerX - (length / 2);
        line_params_h->y1 = centerY;
        line_params_h->x2 = centerX + (length / 2);
        line_params_h->y2 = centerY;
        line_params_h->line_width = 2;
        line_params_h->line_color = (NvOSD_ColorParams) {1, 0, 0, 1}; // RGBA Red

        // Vertical line
        NvOSD_LineParams *line_params_v = &display_meta->line_params[display_meta->num_lines++];
        line_params_v->x1 = centerX;
        line_params_v->y1 = centerY ; //- (length / 2);
        line_params_v->x2 = centerX;
        line_params_v->y2 = centerY + (length / 2);
        line_params_v->line_width = 2;
        line_params_v->line_color = (NvOSD_ColorParams) {1, 0, 0, 1}; //

        int bracketSize = 30; // Size of each bracket arm
        int bracketThickness = 2; // Line thickness of the bracket
        int x_offsetFromCenter = 90; // Offset from the crosshair center
        int offsetFromCenter = 60; // Offset from the crosshair center



        // Top-left bracket
        instance->addLineToDisplayMeta(display_meta, centerX - x_offsetFromCenter, centerY - offsetFromCenter, centerX - x_offsetFromCenter + bracketSize, centerY - offsetFromCenter);
        instance->addLineToDisplayMeta(display_meta, centerX - x_offsetFromCenter, centerY - offsetFromCenter, centerX - x_offsetFromCenter, centerY - offsetFromCenter + bracketSize);

        // Top-right bracket
        instance->addLineToDisplayMeta(display_meta, centerX + x_offsetFromCenter, centerY - offsetFromCenter, centerX + x_offsetFromCenter - bracketSize, centerY - offsetFromCenter);
        instance->addLineToDisplayMeta(display_meta, centerX + x_offsetFromCenter, centerY - offsetFromCenter, centerX + x_offsetFromCenter, centerY - offsetFromCenter + bracketSize);

        // Bottom-left bracket
        instance->addLineToDisplayMeta(display_meta, centerX - x_offsetFromCenter, centerY + offsetFromCenter, centerX - x_offsetFromCenter + bracketSize, centerY + offsetFromCenter);
        instance->addLineToDisplayMeta(display_meta, centerX - x_offsetFromCenter, centerY + offsetFromCenter, centerX - x_offsetFromCenter, centerY + offsetFromCenter - bracketSize);

        // Bottom-right bracket
        instance->addLineToDisplayMeta(display_meta, centerX + x_offsetFromCenter, centerY + offsetFromCenter, centerX + x_offsetFromCenter - bracketSize, centerY + offsetFromCenter);
        instance->addLineToDisplayMeta(display_meta, centerX + x_offsetFromCenter, centerY + offsetFromCenter, centerX + x_offsetFromCenter, centerY + offsetFromCenter - bracketSize);



        nvds_add_display_meta_to_frame(frame_meta, display_meta);
    }
    /*g_print ("Frame Number = %d Number of objects = %d "
            "Vehicle Count = %d Person Count = %d\n",
            frame_number, num_rects, vehicle_count, person_count);*/

    frame_number++;
    return GST_PAD_PROBE_OK;
}

void CameraWidget::setCommonTextParams(NvOSD_TextParams *txt_params) {
    txt_params->font_params.font_name = "Serif";
    txt_params->font_params.font_size = 12;
    txt_params->font_params.font_color.red = 1.0;
    txt_params->font_params.font_color.green = 1.0;
    txt_params->font_params.font_color.blue = 1.0;
    txt_params->font_params.font_color.alpha = 1.0;

    txt_params->set_bg_clr = 1;
    txt_params->text_bg_clr.red = 1.0;
    txt_params->text_bg_clr.green = 0.0;
    txt_params->text_bg_clr.blue = 0.0;
    txt_params->text_bg_clr.alpha = 0.6;
}

// Function to add a line to the display meta
void CameraWidget::addLineToDisplayMeta(NvDsDisplayMeta *display_meta, int x1, int y1, int x2, int y2) {
    NvOSD_LineParams *line_params = &display_meta->line_params[display_meta->num_lines++];
    line_params->x1 = x1;
    line_params->y1 = y1;
    line_params->x2 = x2;
    line_params->y2 = y2;
    line_params->line_width = 2;
    line_params->line_color = (NvOSD_ColorParams){1, 0, 0, 0.8};
}



GstPadProbeReturn CameraWidget::metadata_probe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    CameraWidget* self = reinterpret_cast<CameraWidget*>(user_data);
    GstBuffer *buf = GST_PAD_PROBE_INFO_BUFFER(info);
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);
    QList<ObjectInfo> objectInfos;

    for (auto l_frame = batch_meta->frame_meta_list; l_frame != nullptr; l_frame = l_frame->next) {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *)(l_frame->data);

        for (auto l_obj = frame_meta->obj_meta_list; l_obj != nullptr; l_obj = l_obj->next) {
            NvDsObjectMeta *obj_meta = (NvDsObjectMeta *)(l_obj->data);

            ObjectInfo info;
            info.frameNumber = frame_meta->frame_num;
            info.uniqueId = obj_meta->object_id;
            info.classId = obj_meta->class_id;
            info.bboxLeft = obj_meta->rect_params.left;
            info.bboxTop = obj_meta->rect_params.top;
            info.bboxRight = obj_meta->rect_params.width + obj_meta->rect_params.left;
            info.bboxBottom = obj_meta->rect_params.height + obj_meta->rect_params.top;
            info.confidence = obj_meta->confidence;
            info.trackerState = obj_meta->tracker_confidence;
            info.visibility = obj_meta->rect_params.has_bg_color ? obj_meta->rect_params.bg_color.alpha : 1.0;

            objectInfos.append(info);
        }
    }

    emit self->objectMetadataProcessed(objectInfos);

    /*GstBuffer *buf = (GstBuffer *)info->data;
    NvDsMetaList * l_frame = NULL;
    NvDsMetaList * l_obj = NULL;
    NvDsObjectMeta *obj_meta = NULL;
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);

    // Iterate over each frame's metadata in the batch
    for (NvDsMetaList *l_frame = batch_meta->frame_meta_list; l_frame != NULL; l_frame = l_frame->next) {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *)(l_frame->data);

        int offset = 0;
        for (l_obj = frame_meta->obj_meta_list; l_obj != NULL; l_obj = l_obj->next) {
                obj_meta = (NvDsObjectMeta *) (l_obj->data);

                guint left  =  obj_meta->rect_params.left;
                guint top  = obj_meta->rect_params.top;
                guint width  = obj_meta->rect_params.width;
                guint height  = obj_meta->rect_params.height;
                //guint object_id  = obj_meta->object_id;
                guint class_id  = obj_meta->class_id;
                guint confidence  = obj_meta->confidence;
                //g_print ("class_id = %d "  "confidence = %d left = %d   top = %d\n",
            //     class_id,  confidence, left, top);
                     // Emit signal to Qt application (ensure thread safety)
                //QMetaObject::invokeMethod(qtObject, "emitMetadataSignal", Qt::QueuedConnection, Q_ARG(NvDsMetaList*, l_obj));

                //QMetaObject::invokeMethod(this, "objectMetadataReceived", Qt::QueuedConnection, Q_ARG(QList<ObjectInfo>, objectList));

        }
        QList<ObjectInfo> objects;

        // Your existing code to process metadata and populate objects list

        // Emit the signal with the processed object metadata
        emit self->objectMetadataProcessed(objects);

        // Iterate over each object's metadata in the frame
        for (NvDsMetaList *l_obj = frame_meta->obj_meta_list; l_obj != NULL; l_obj = l_obj->next) {
            NvDsObjectMeta *obj_meta = (NvDsObjectMeta *)(l_obj->data);
            // Process or emit signal with obj_meta for further processing
            // For example, emit a Qt signal:
            //emit signalNewObjectMeta(*obj_meta);
        }
    }*/
    return GST_PAD_PROBE_OK;
}

void CameraWidget::setTargetPosition(const QPoint& position) {
    qDebug() << "Setting target position. CameraWidget instance:" << this;
    //qDebug() << "Current target position:" << targetPosition << ", New position:" << position;

    //targetPosition = position;

    //qDebug() << "New target position set successfully to:" << targetPosition;
    //update();  // Trigger a repaint
}




/*void CameraWidget::paintEvent(QPaintEvent *) {
    //qDebug() << "Paint event triggered. Target position:" << targetPosition;


    QPainter painter(this);
    //painter.setRenderHint(QPainter::Antialiasing);
    if (!currentFrame.isNull()) {
        painter.drawImage(0, 0, currentFrame.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    QPoint center(width() / 2, height() / 2);
    painter.drawEllipse(QPoint(500, 200), 50, 50);
    // Only draw the line if the target position is valid (not at (0,0))




    // Additional HUD elements can be drawn here as needed
}*/

void CameraWidget::displayFrame(const QImage& frame) {
    // qDebug() << "displayFrame triggered. ";
    currentFrame = frame;
    update(); // Schedule a repaint
}

/*GstPadProbeReturn CameraWidget::toggle_pgie_probe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    CameraWidget *cameraWidget = reinterpret_cast<CameraWidget*>(user_data);
    if (!cameraWidget) {
        g_printerr("Failed to get CameraWidget instance.\n");
        return GST_PAD_PROBE_REMOVE;
    }
    g_print ("entering toggle_pgie_probe method... \n");
        // Toggle the inclusion of "pgie" based on some condition (e.g., joystick button)
    //bool joystick_button_pressed = true; // Replace this with your joystick button state
    if (cameraWidget->enableDetection && !cameraWidget->pgie) {
        g_print ("entering the if statment... \n");
        // Add "pgie" to the pipeline
        gst_bin_add(GST_BIN(pipeline), cameraWidget->pgie);
        // Link "pgie" with existing elements
        gst_element_link_many(cameraWidget->streammux, cameraWidget->pgie, cameraWidget->vidconv, cameraWidget->nvosd, cameraWidget->transform, cameraWidget->sink, NULL);
    } else if (!cameraWidget->enableDetection && cameraWidget->pgie) {
        g_print ("entering the else statment... \n");
        gst_element_set_state(pgie, GST_STATE_NULL);
        gst_bin_remove(GST_BIN(pipeline), cameraWidget->pgie);
        cameraWidget->pgie = NULL;
        // Relink elements without "pgie"
        gst_element_link_many(cameraWidget->streammux, cameraWidget->vidconv, cameraWidget->nvosd, cameraWidget->transform, cameraWidget->sink, NULL);
    }
    return GST_PAD_PROBE_OK;
}

 // Function to set up pad probe
void CameraWidget::setup_toggle_pgie_probe() {
    GstPad *sinkpad = gst_element_get_static_pad(this->streammux, "sink");
    gst_pad_add_probe(sinkpad, GST_PAD_PROBE_TYPE_BUFFER, [](GstPad *pad, GstPadProbeInfo *info, gpointer user_data) -> GstPadProbeReturn {
        // Logging the values of pad, info, and user_data
        g_print("Pad: %p, Info: %p, User_data: %p\n", pad, info, user_data);

        CameraWidget *cameraWidget = reinterpret_cast<CameraWidget*>(user_data);
        if (!cameraWidget) {
            g_printerr("Failed to get CameraWidget instance.\n");
            return GST_PAD_PROBE_REMOVE;
        }

        return cameraWidget->toggle_pgie_probe(pad, info, user_data);
    }, this, NULL);
    gst_object_unref(sinkpad);
}*/


void CameraWidget::toggleDetection(bool enabled) {
    // Logic to enable/disable detection
    this->enableDetection = enabled;
    std::cout << "Detection toggled to: " << (enabled ? "ON" : "OFF");
}

void CameraWidget::toggleTracking(bool enabled) {
    this->enableTracking = enabled;
    // Logic to enable/disable tracking
    std::cout << "Tracking toggled to: " << (enabled ? "ON" : "OFF");
}

gboolean CameraWidget::busCall(GstBus *bus, GstMessage *msg, gpointer data) {
    GMainLoop *loop = static_cast<GMainLoop *>(data);
    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
        g_print("End of stream\n");
        g_main_loop_quit(loop);
        break;
    case GST_MESSAGE_ERROR: {
        gchar *debug;
        GError *error;
        gst_message_parse_error(msg, &error, &debug);
        g_printerr("ERROR from element %s: %s\n", GST_OBJECT_NAME(msg->src), error->message);
        if (debug)
            g_printerr("Error details: %s\n", debug);
        g_free(debug);
        g_error_free(error);
        g_main_loop_quit(loop);
        break;
    }
    default:
        break;
    }
    return TRUE;
}
/* Wait until error or EOS */
//bus = gst_element_get_bus(pipeline);
//msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
//msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

/* Free resources */
//if (msg != NULL)
//     gst_message_unref(msg);
//gst_object_unref(bus);
// gst_element_set_state(pipeline, GST_STATE_NULL);
//gst_object_unref(pipeline);
