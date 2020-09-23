import QtQuick 2.0

Item {
    Rectangle {
        width: 200
        height: 200
        color: "red"
        
        anchors.centerIn: parent
        
        NumberAnimation on rotation {
            from: 0
            to: 360;
            loops: Animation.Infinite;
            duration: 1000;
        }
    }
}