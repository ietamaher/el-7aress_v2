#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H


#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPainter>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QImage>

#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <gst/app/gstappsink.h>


#include <glib.h>
#include <stdio.h>
#include <cuda_runtime_api.h>
#include "gstnvdsmeta.h"
#include <QMetaType>
#include <QList>
#include "objectinfo.h"
#include <QDebug>
//#include "trackingmanager.h"



#define USE_OPENCV 0

#define PGIE_CLASS_ID_VEHICLE 0
#define PGIE_CLASS_ID_PERSON 2

/* The muxer output resolution must be set if the input streams will be of
 * different resolution. The muxer will scale all the input frames to this
 * resolution. */
#define MUXER_OUTPUT_WIDTH 1280
#define MUXER_OUTPUT_HEIGHT 720

/* Muxer batch formation timeout, for e.g. 40 millisec. Should ideally be set
 * based on the fastest source's framerate. */
#define MUXER_BATCH_TIMEOUT_USEC 40000
#define MAX_DISPLAY_LEN 200

class CameraWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit CameraWidget(QWidget *parent = nullptr);
    ~CameraWidget();

    void startCamera();
    /*void setup_toggle_pgie_probe();
    GstPadProbeReturn toggle_pgie_probe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data); */
    GstPadProbeReturn interval_property_probe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
    //GstPadProbeReturn metadata_probe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
    void setCommonTextParams(NvOSD_TextParams *txt_params);
    void setTargetPosition(const QPoint& position);
    void displayFrame(const QImage& frame);

    static int frame_number;
    static constexpr gchar pgie_classes_str[4][32] = {
        "Vehicle", "TwoWheeler", "Person", "Roadsign"
    };
protected:
    //void paintEvent(QPaintEvent *event) override;

private:
    //TrackingManager *trackingManager;
    //QPoint targetPosition;
    static GstFlowReturn on_new_sample(GstAppSink *sink, gpointer data);
    static GstPadProbeReturn metadata_probe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
    static GstPadProbeReturn osd_sink_pad_buffer_probe (GstPad * pad, GstPadProbeInfo * info, gpointer u_data);
    static gboolean busCall(GstBus *bus, GstMessage *msg, gpointer data);
    void addLineToDisplayMeta(NvDsDisplayMeta *display_meta, int x1, int y1, int x2, int y2);
    GstElement *glupload, *glcolorconvert, *pipeline, *source, *sink, *pgie, *tracker, *streammux, *nvvidconvsrc, *vidconv, *nvosd, *transform, *capsfilter1, *capsfilter2, *decoder;
    GstCaps *caps1, *caps2;
    guint bus_watch_id;
    GstBus *bus;
    GstMessage *msg;
    int current_device = -1;
    GMainLoop *loop;

    bool enableDetection;
    bool enableTracking;

    QImage currentFrame;
    QString currentBurstMode;
    bool currentTrackingState;
    bool currentDetectionState;
    bool currentStabState;
    double currentSpeed;
    int currentincr_;
    double currentAzimuth;
    int currentincr_1;
    double currentInput1;

signals:
    void objectMetadataReceived(const QList<ObjectInfo>& objects);
    void frameReady(const QImage& frame);
    void objectMetadataProcessed(const QList<ObjectInfo>& objects);
public slots:
    //void HandleShutdown(); // Slot to handle cleanup and shutdown
    void toggleDetection(bool enabled);
    void toggleTracking(bool enabled);
    void setSettingParameters(const QString& burstMode, bool trackingState, bool detectionState, bool stabState, double speed);
    void setMotorParameters(int incr_, double azimuth);
    void setPlcParameters(int incr_, bool in1);

};

#endif // CAMERAWIDGET_H
