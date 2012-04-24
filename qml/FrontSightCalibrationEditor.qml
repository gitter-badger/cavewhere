// import QtQuick 2.0 // to target S60 5th Edition or Maemo 5
import QtQuick 2.0
import Cavewhere 1.0
import "Utils.js" as Utils

CheckableGroupBox {
    id: calibrationEditor

    property Calibration calibration

    anchors.margins: 3
    contentHeight: frontSightContent.height
    text: "<b>Front Sights</b>"
    checked: calibration.frontSights

    onCalibrationChanged: {
        checked = calibration.frontSights
    }

    onCheckedChanged: {
        calibration.frontSights = checked
    }

    Column {
        id: frontSightContent
        anchors.left: parent.left
        anchors.right: parent.right

        Row {
            spacing: 3

            LabelWithHelp {
                id: compassCalibrationLabel
                text: "Compass calibration"
                helpArea: compassCalibarationHelpArea
            }

            ClickTextInput {
                id: clinoCalInput
                text: Utils.fixed(calibration.frontCompassCalibration, 2)

                onFinishedEditting: {
                    calibration.frontCompassCalibration = newText
                }
            }
        }

        HelpArea {
            id: compassCalibarationHelpArea
            anchors.left: parent.left
            anchors.right: parent.right
            text: "<p>Front sight compass calibration allows you to correct an instrument that's off.
The calibration is added to uncorrected value
(the value you read off the instrument) to find the true value.</p>
<b>UncorrectedValue + Calibration = TrueValue</b>
<p> For example, the reading in the cave was 180°. Your instrument is off by -2°. The bearing is really
182° instead of 180 (because your insturment is subtracting 2° at every shot).  So you need to enter 2
for the calibration to correct it. UncorrectedValue = 180°,
Calibration = 2°, so 180° + (2°) = 182° </p>"
        }

        Row {
            spacing: 3

            LabelWithHelp {
                id: clinoCalibrationLabel
                text: "Clino calibration"
                helpArea: clinoCalibarationHelpArea
            }

            ClickTextInput {
                id: compassCalInput
                text: Utils.fixed(calibration.frontClinoCalibration, 2)

                onFinishedEditting: {
                    calibration.frontClinoCalibration = newText
                }
            }
        }



        HelpArea {
            id: clinoCalibarationHelpArea
            anchors.left: parent.left
            anchors.right: parent.right
            text: "<p>Front sight clino calibration allows you to correct an instrument that's off.
The calibration is added to uncorrected value
(the value you read off the instrument) to find the true value.</p>
<b>UncorrectedValue + Calibration = TrueValue</b>
<p> For example, the reading in the cave was +4°. Your instrument is off by +1°. The bearing is really
+3° instead of +4° (because your insturment is adding extra 1° at every shot).  So you need to enter -1
for the calibration to correct it. UncorrectedValue = +4°,
Calibration = -1°, so +4° + (-1°) = +3° </p>"
        }
    }
}
