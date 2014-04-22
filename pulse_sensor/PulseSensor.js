/**
 * Copyright (c) 2011-2013 Jeff Hoefs <soundanalogous@gmail.com>
 * Released under the MIT license. See LICENSE file for details.
 */

JSUTILS.namespace('BO.custom.PulseSensor');

BO.custom.PulseSensor = (function () {

    var PulseSensor;

    // private static constants:
    var PULSE_SENSOR = 0x0E;

    // dependencies
    var EventDispatcher = JSUTILS.EventDispatcher,
        IOBoardEvent = BO.IOBoardEvent,
        Event = JSUTILS.Event;

    /**
     * Pulse Sensor.
     *
     * @exports PulseSensor as BO.custom.PulseSensor
     * @class Creates an interface to a pulse sensor. This object
     * requires firmware other than AdvancedFirmata to be uploaded to the I/O 
     * board.
     * See Breakout/custom_examples/rfid_example1.html and rfid_example2.html 
     * for example applications.
     * @constructor
     * @param {IOBoard} board A reference to the IOBoard instance
     * @param {Number} sensorId The ID assigned to the sensor in the firmware
     * running on the IOBoard (default = 0x0E)
     */
    PulseSensor = function (board, sensorId) {
        "use strict";

        this.name = "PulseSensor";

        this._sensorId = sensorId || PULSE_SENSOR;
        this._board = board;
        this._evtDispatcher = new EventDispatcher(this);

        board.addEventListener(IOBoardEvent.SYSEX_MESSAGE, this.onSysExMessage.bind(this));         
    };

    PulseSensor.prototype = {

        // private methods:
        /**
         * @private
         */
        onSysExMessage: function (event) {
            var message = event.message;

            if (message[0] != this._sensorId) {
                return;
            } else {
                this.processSensorData(message);
            }
        },
        
        /**
         * @private
         */
        processSensorData: function (data) {
            // The midi-based Firmata protocol sends data in 7-bit chunks
            // they need to be reassembled into bytes.
            // data[0] is the device ID (PULSE_SENSOR)
            var dataType = this._board.getValueFromTwo7bitBytes(data[1], data[2]),
                sensorValLSB,
                sensorValMSB,
                sensorVal;

            if (data.length === 7) {
                sensorValLSB = this._board.getValueFromTwo7bitBytes(data[3], data[4]);
                sensorValMSB = this._board.getValueFromTwo7bitBytes(data[5], data[6]);

                sensorVal = (sensorValMSB << 8) + sensorValLSB;

                this.dispatchEvent(new Event(Event.CHANGE), {id: dataType, value: sensorVal});
            } else {
                console.log("PulseSensor: insufficient data received");
            }

        },

        // public methods

        startReading: function () {
            var data = [];
            data[0] = 1;

            this._board.sendSysex(PULSE_SENSOR, data);
        },

        stopReading: function() {
            var data = [];
            data[0] = 2;
            
            this._board.sendSysex(PULSE_SENSOR, data);
        },
        
        /* implement EventDispatcher */
        
        /**
         * @param {String} type The event type
         * @param {Function} listener The function to be called when the event is fired
         */
        addEventListener: function (type, listener) {
            this._evtDispatcher.addEventListener(type, listener);
        },
        
        /**
         * @param {String} type The event type
         * @param {Function} listener The function to be called when the event is fired
         */
        removeEventListener: function (type, listener) {
            this._evtDispatcher.removeEventListener(type, listener);
        },
        
        /**
         * @param {String} type The event type
         * return {boolean} True is listener exists for this type, false if not.
         */
        hasEventListener: function (type) {
            return this._evtDispatcher.hasEventListener(type);
        },
        
        /**
         * @param {Event} type The Event object
         * @param {Object} optionalParams Optional parameters to assign to the event object.
         * return {boolean} True if dispatch is successful, false if not.
         */     
        dispatchEvent: function (event, optionalParams) {
            return this._evtDispatcher.dispatchEvent(event, optionalParams);
        }       

    };

    PulseSensor.SIGNAL_ID = 0x00;
    PulseSensor.BPM_ID = 0x01;
    PulseSensor.IBI_ID = 0x02;

    // document events

    /**
     * The change event is dispatched when new pulse sensor data is received.
     * @name PulseSensor#change
     * @type JSUTILS.Event.CHANGE
     * @event
     * @param {BO.custom.PulseSensor} target A reference to the PulseSensor 
     * object.
     * @param {Object} id: The type of data (Signal_ID, BPM_ID or BMI_ID).
     * value: The value of the associated id.
     */

    return PulseSensor;

}());
