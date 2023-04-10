import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.VirtualKeyboard 2.2

import MobileNetwork 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 640
    height: 480
    title: qsTr("Mobile Network")

    MobileNetwork {
        id: mobileNetwork_id
        onSyncNetworkState: {
//            console.log("fulinux state = " + state)
            if (state === 1)
                switch_btn.checked = true
            else
                switch_btn.checked = false
        }
    }

    Rectangle{
        id:titlebar
        visible: true
        width: parent.width
        height: 150
        z:3

        ColumnLayout {
            anchors.fill: parent
            spacing: 8

            Rectangle{
                height:30
            }

            RowLayout {
                spacing: 50

                Text {
                    Layout.leftMargin: 40
                    font.pixelSize: 30
                    Layout.preferredWidth: titlebar.width * 2/3
                    text: qsTr("Mobile Network")
                }

                Switch {
                    id:switch_btn
                    Layout.fillWidth: true
                    checked : true
                    onClicked : {
                        if(switch_btn.checked) {
                            mobileNetwork_id.sendSignalFromQML(1);
                        } else {
                            mobileNetwork_id.sendSignalFromQML(2);
                        }
                    }
                }
            }

            Rectangle{
                height:50
                Layout.fillWidth: true
            }

            Rectangle{
                height:20
                Layout.fillWidth: true
                color:"gainsboro"
            }
        }
    }

    NumberAnimation {
        id:ani1
        target: listview_id
        property: "contentY"
        duration: 200
        to:-listview_id.headerItem.height
        running: false
        easing.type: Easing.InOutQuad
    }

    NumberAnimation {
        id:ani2
        target: listview_id
        property: "contentY"
        duration: 200
        to:-titlebar.height
        running: false
        easing.type: Easing.InOutQuad
    }

    Component {
        id: headerView
        Item {
            width: listview_id.width
            height: 200
            Label{
                id:label
                text: qsTr("Network Information")
                font.pixelSize: 20
//                color:"white"
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 8
                Rectangle{
                    id:line_id
                    height:1
                    width: 600
                    anchors.bottom: parent.bottom
                    color:"gainsboro"
                }
            }
        }
    }

    //footer view
    Component {
        id: footerView
        Item {
            id: footerRootItem
            width: listview_id.width
            height: 30
            signal add()

            Button {
                text: qsTr("    Exit    ")
                background:Rectangle{
                    anchors.fill: parent
                    color: "red"
                }
                onClicked: {
                    mobileNetwork_id.exitProgram()
                    Qt.quit()
                }
            }
        }
    }

    ListView{
        id:listview_id
        anchors.fill: parent
        anchors.left: parent.left
        anchors.leftMargin: 40
        anchors.right: parent.right
        anchors.rightMargin: 40
        focus: true

        model: mobileNetwork_id.infoList

        onContentYChanged:{
//            if(listview_id.contentY < -titlebar.height){
//                titlebar.opacity = 1-(-listview_id.contentY - titlebar.height)/100.
//                titlebar.y = -listview_id.contentY - titlebar.height
//            }
//            else{
//                item.opacity = 1
//                titlebar.y = 0
//            }
        }

        onMovementEnded: {
//            if(listview_id.contentY < -listview_id.headerItem.height/2.){
//                ani1.running = true
//            } else if(listview_id.contentY < -titlebar.height){
//                ani2.running = true
//            }
        }

        header: headerView

        delegate: Item {
            id: wrapper
            width: listview_id.width
            height:50

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    wrapper.ListView.view.currentIndex = index
                }

                onDoubleClicked: {
                }
            }

            ColumnLayout {
                spacing: 8
                RowLayout {
//                    anchors.left: parent.left
//                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 100
                    anchors.topMargin: 3

                    Text {
                        text: mobileNetwork_id.getInfoAt(index)
                        color: wrapper.ListView.isCurrentItem ? "red":"black"
                        font.pixelSize: wrapper.ListView.isCurrentItem ? 24:20
                        Layout.fillWidth: true
                        Layout.topMargin: 3
                    }
                }

                Rectangle{
                    id:line_id
                    height:1
//                    width: wrapper.width
                    width: 600
                    color:"gainsboro"
                }
            }
        }

//        footer: Rectangle{
//            id:foot
//            color: "green"
//            width: parent.width
//            height: 50
//        }
        footer: footerView

        function addOne() {
        }

        Component.onCompleted: {
            var t_h = listview_id.model.count * 50 + titlebar.height
            if(t_h < listview_id.height){
                listview_id.footerItem.height = listview_id.height - t_h
            }

            listview_id.footerItem.add.connect(listview_id.addOne)
        }

        ScrollIndicator.vertical: ScrollIndicator { }
    }

    InputPanel {
        id: inputPanel
        z: 99
        x: 0
        y: window.height
        width: window.width

        states: State {
            name: "visible"
            when: inputPanel.active
            PropertyChanges {
                target: inputPanel
                y: window.height - inputPanel.height
            }
        }
        transitions: Transition {
            from: ""
            to: "visible"
            reversible: true
            ParallelAnimation {
                NumberAnimation {
                    properties: "y"
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}
