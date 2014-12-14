    #include <toneAC.h>
     
    void setup() {} // Nothing to setup, just start playing!
     
    void loop() {
        toneAC(1000); // Play the frequency (150 Hz to 15 kHz).
        delay(1);     // Wait 1 ms so you can hear it.
      toneAC(0); // Turn off toneAC, can also use noToneAC().
      delay(1000);
    }

