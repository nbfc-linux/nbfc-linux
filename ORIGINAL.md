Notes about the original NBFC project
-------------------------------------

- StagWare.FanControl/FanControl.cs:
  - ```var speeds = new float[this.fans.Length];```
    This variable is never used

- StagWare.FanControl/FanControl.cs/Fan.cs:
  - ```GetCurrentSpeed()```
    Maybe rename this to ```ReadCurrentSpeed()/UpdateCurrentSpeed()` as this function
    isn't just a simple property getter.

  - Make `CriticalTemperatureOffset` configurable?

  - Maybe delete `requestedSpeeds[]` and rely only on the state inside Fan.cs

  - HandleCritialTemperature():
    Test on `if (critialMode.enabled)` can be dropped.

    ```
    if (this.CriticalModeEnabled
        && (temperature < (this.criticalTemperature - CriticalTemperatureOffset)))
    {
        this.CriticalModeEnabled = false;
    }
    else if (temperature > this.criticalTemperature)
    {
        this.CriticalModeEnabled = true;
    }

    --->

    if (temperature > my.criticalTemperature)
      my.isCritical = true;
    else if (temperature < (my.criticalTemperature - my.criticalTemperatureOffset))
      my.isCritical = false;

    ```
    
- Core/StagWare.FanControl/TemperatureThresholdManager.cs:
  - ```ResetCurrentThreshold()``` broken?
  - Rename ```double cpuTemperature -> double temperature``` (do we really care if it is a CPU?)
      
  - AutoSelectThreshold():
    - Isn't it safer to begin with the *highest* fan speed? (is this really the case?)
    - Isn't it better to directly jump to the next threshold instead of using steps? (this is what I do)
    - If fan mode is fixed speed, AutoSelectThreshold() is not called.
      It would make sense to do so, so we get the right temperature when swichting back to AutoMode.

