import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.1
//import QtQuick.Dialogs 1.0
import tel_sdk 1.0

Window {
    width: 540
    height: 950
    visible: true
    MouseArea {
            anchors.fill: parent
            onClicked: {
                //单击鼠标调用begin信号函数
            }
        }

    title: qsTr("Telephone handset")
    ListModel {
        id: recentCallsListModel
        ListElement {
            number: "+138-8888-8888"
            duration: "00:10"
        }
    }

    Dialog {
        id: dialog
        property date currentDate: new Date()
        standardButtons: StandardButton.Cancel
        contentItem: FocusScope {
            implicitWidth: 400
            implicitHeight: 100
            Rectangle {
                anchors.fill: parent
                color: "lightskyblue"
            }
            ColumnLayout {
                anchors.fill: parent
                Item {
                    Layout.fillWidth: true
                    implicitHeight: 50
                    Text {
                        text: qsTr("Dialing...\n")+textInput.text
                        color: "navy"
                        anchors.centerIn: parent
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
                Button {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: qsTr("Cancel")
                    
                    focus: true
                    onClicked: {
                        var closeDate = new Date()
                        dialog.close()
						dial_stop()
                        var durationDate = new Date(closeDate-dialog.currentDate)
                        // FIXME: timezones problem:
                        recentCallsListModel.append({"number": textInput.text, "duration":durationDate.toTimeString()})
                    }
                }
            }
        }
    }

	Dialog {
        id: dialog1
        property date currentDate: new Date()
        standardButtons: StandardButton.Cancel
        contentItem: FocusScope {
            implicitWidth: 400
            implicitHeight: 100
            Rectangle {
                anchors.fill: parent
                color: "lightskyblue"
            }
            ColumnLayout {
                anchors.fill: parent
                Item {
                    Layout.fillWidth: true
                    implicitHeight: 50
                    Text {
                        text: qsTr("Dialing...\n")+textInput1.text
                        color: "navy"
                        anchors.centerIn: parent
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
				Button {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: qsTr("jieting")
                    //isDefault: true
                    focus: true
                    onClicked: {
                        var closeDate = new Date()
                        dialog1.close()
						root.tel_answer()
                        dialog2.visible=true
                        var durationDate = new Date(closeDate-dialog1.currentDate)
                        // FIXME: timezones problem:
                        recentCallsListModel.append({"number": textInput1.text, "duration":durationDate.toTimeString()})

                    }
                }
                Button {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: qsTr("Cancel")
                    
                    focus: true
                    onClicked: {
                        var closeDate = new Date()
                        dialog1.close()
						dial_stop()
                        var durationDate = new Date(closeDate-dialog.currentDate)
                        // FIXME: timezones problem:
                        recentCallsListModel.append({"number": textInput1.text, "duration":durationDate.toTimeString()})
                    }
                }
            }
        }
    }
	Dialog {
        id: dialog2
        property date currentDate: new Date()
        standardButtons: StandardButton.Cancel
        contentItem: FocusScope {
            implicitWidth: 400
            implicitHeight: 100
            Rectangle {
                anchors.fill: parent
                color: "lightskyblue"
            }
            ColumnLayout {
                anchors.fill: parent
                Item {
                    Layout.fillWidth: true
                    implicitHeight: 50
                    Text {
                        text: qsTr("Dialing...\n")+textInput1.text
                        color: "navy"
                        anchors.centerIn: parent
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
                Button {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: qsTr("Cancel")
                    
                    focus: true
                    onClicked: {
                        var closeDate = new Date()
                        dialog2.close()
						dial_stop()
                        var durationDate = new Date(closeDate-dialog.currentDate)
                        // FIXME: timezones problem:
                        recentCallsListModel.append({"number": textInput1.text, "duration":durationDate.toTimeString()})
                    }
                }
            }
        }
    }
    ColumnLayout {
        anchors.fill: parent
        ListView {
            Layout.fillWidth: true
            implicitHeight: 150
            model: recentCallsListModel
            delegate: Item {
                width: parent.width
                height: 50
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 20
                    Text {
                        id: numberText
                        text: number
                    }
                    // FIXME: appended item layout problem:
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                    Text {
                        color: "grey"
                        text: duration
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            textInput.insert(0,numberText.text)
                        }
                    }
                }
            }
        }

        TextInput {
            id: textInput
            Layout.fillWidth: true
            Layout.fillHeight: true
            selectByMouse: true
            inputMask: "00000000000"
            inputMethodHints: Qt.ImhDialableCharactersOnly
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 40
            font.bold: true
            font.family: "monospace"
        }
		Text  {
            id: textInput1
            
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: "1"
                onClicked: {
                    textInput.insert(textInput.cursorPosition,text)
                    textInput.cursorPosition+=1
                }
            }
            Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: "2"
                onClicked: {
                    textInput.insert(textInput.cursorPosition,text)
                    textInput.cursorPosition+=1
                }
            }
            Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: "3"
                onClicked: {
                    textInput.insert(textInput.cursorPosition,text)
                    textInput.cursorPosition+=1
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: "4"
                onClicked: {
                    textInput.insert(textInput.cursorPosition,text)
                    textInput.cursorPosition+=1
                }
            }
            Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: "5"
                onClicked: {
                    textInput.insert(textInput.cursorPosition,text)
                    textInput.cursorPosition+=1
                }
            }
            Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: "6"
                onClicked: {
                    textInput.insert(textInput.cursorPosition,text)
                    textInput.cursorPosition+=1
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: "7"
                onClicked: {
                    textInput.insert(textInput.cursorPosition,text)
                    textInput.cursorPosition+=1
                }
            }
            Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: "8"
                onClicked: {
                    textInput.insert(textInput.cursorPosition,text)
                    textInput.cursorPosition+=1
                }
            }
            Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: "9"
                onClicked: {
                    textInput.insert(textInput.cursorPosition,text)
                    textInput.cursorPosition+=1
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Button {
                enabled: false
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: "*"
                onClicked: {
                    textInput.insert(textInput.cursorPosition,text)
                    textInput.cursorPosition+=1

                }
            }
            Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: "0"
                onClicked: {
                    textInput.insert(textInput.cursorPosition,text)
                    textInput.cursorPosition+=1
                }
            }
            Button {
                enabled: false
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: "#"
                onClicked: {
                    textInput.insert(textInput.cursorPosition,text)
                    textInput.cursorPosition+=1
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            id: testButton
            Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: qsTr("Dial")
                ///enabled: textInput.acceptableInput
                onClicked: {
                    dialog.visible=true
                    root.cppSlotA(textInput.text)
                }
            }
            Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: qsTr("Reset")
                onClicked: {
                    textInput.remove(0,textInput.length)
                    textInput.cursorPosition=0
                }
            }
        }
	Timer {
            id: timer
            interval: 20
            running: true
            repeat: true
            property int returnedValue: 0
            onTriggered: {
                //console.log("Loop iteration every 20ms");
                returnedValue = 12;
				flag();
            }
//            onReturnedValueChanged: {
//                timer.stop();
//                console.log("Stop loop wth:", returnedValue);
//            }
        }

    }
	function  flag()
    {
        //dialog.visible=true
        root.display()
        //console.log("QML get message:",mmsg);
    }
    function  test(msg)
    {
        //dialog.visible=true
        root.cppSlotB(msg,1)
        console.log("QML get message:",msg);
    }
	function dial_stop()
	{
		root.dial_stop()
		console.log("QML get message stop stop stop");
	}
	function  test1(msg)
    {
        dialog1.visible=true
		textInput1.text = msg;
        console.log("QML get message222222222222:",msg);
    }
    function  stop(msg)
    {
        dialog.close()
        dialog1.close()
        dialog2.close()
        dial_stop()
        console.log("QML get message5555555555:",msg);
    }
    Tel_sdk
    {
        id: root
    }

}

