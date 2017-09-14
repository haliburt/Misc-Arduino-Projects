void speech_init(void) {
  bvsp.keepAlive();
  bvsp.receive();
  digitalWrite(BVS_RUNNING, bvsp.isBVSRunning());
  digitalWrite(BVS_DATA_FWD, bvsp.isDataFwdRunning());

  if (bvsp.isSREAvailable())
  {
    digitalWrite(BVS_SRE, HIGH);

    if (!bvsm.isRecording)
    {
      bvsm.setAudioInput(BVSM_AUDIO_INPUT, EXTERNAL);
      bvsm.startRecording();
    }

    // Checks if the BVSMic class has available samples
    if (bvsm.available)
    {
      // Makes sure the inbound mode is STREAM_MODE before
      // transmitting the stream
      if (bvsp.inboundMode == FRAMED_MODE)
        bvsp.setInboundMode(STREAM_MODE);

      int bytesRead = bvsm.read(audioBuffer, AUDIO_BUFFER_SIZE);

      bvsp.sendStream(audioBuffer, bytesRead);
    }
  }
  else
  {
    // There is no SRE available, turns off the SRE and ACT_PERIOD LEDs
    digitalWrite(BVS_SRE, LOW);
    digitalWrite(BVS_ACT_PERIOD, LOW);

    if (bvsm.isRecording)
      bvsm.stopRecording();
  }
}

// Handles the frameReceived event
void BVSP_frameReceived(byte dataType, int payloadSize)
{
  switch (dataType)
  {
    case DATA_TYPE_BYTE:
      digitalWrite(BVS_ACT_PERIOD, bvsp.getReceivedByte());
      break;
    case DATA_TYPE_STRING:
      if (bvsp.getReceivedString(stringBuffer, STRING_BUFFER_SIZE) != 0);
      {
        if (strcmp(stringBuffer, "fanOn") == 0) {
          fan("on");
        }
        else if (strcmp(stringBuffer, "fanOff") == 0) {
          fan("off");
        }
        else if (strcmp(stringBuffer, "heatOn") == 0) {
          heater("on");
        }
        else if (strcmp(stringBuffer, "heatOff") == 0) {
          heater("off");
        }
        else if (strcmp(stringBuffer, "bothOn") == 0) {
          heater("on");
          fan("on");
        }
        else if (strcmp(stringBuffer, "bothOff") == 0) {
          heater("off");
          fan("off");
        }
        else if (strcmp(stringBuffer, "regOn") == 0) {
          auto_reg("on");
        }
        else if (strcmp(stringBuffer, "regOff") == 0) {
          auto_reg("off");
        }
      }
        break;
  }
}
