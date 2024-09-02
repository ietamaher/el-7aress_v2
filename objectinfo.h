#ifndef OBJECTINFO_H
#define OBJECTINFO_H

#include <QMetaType>

struct ObjectInfo {
    unsigned long frameNumber;  // Frame number in which the object is detected or tracked
    unsigned long uniqueId;     // Unique identifier for the object
    unsigned int classId;       // Class ID of the detected object (e.g., car, person)
    float bboxLeft;             // Left coordinate of the bounding box
    float bboxTop;              // Top coordinate of the bounding box
    float bboxRight;            // Right coordinate of the bounding box
    float bboxBottom;           // Bottom coordinate of the bounding box
    float confidence;           // Confidence score of the detection or tracking
    int trackerState;           // State of the tracker for this object
    float visibility;           // Visibility ratio of the object

};

Q_DECLARE_METATYPE(ObjectInfo)

#endif // OBJECTINFO_H
