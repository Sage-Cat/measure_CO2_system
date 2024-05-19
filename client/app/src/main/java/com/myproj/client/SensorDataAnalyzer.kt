package com.myproj.client

import android.util.Log
import com.myproj.client.network.ApiClient

class SensorDataAnalyzer {

     fun onDataReceived(samples: List<CO2Sample>) {

        Log.d("SensorDataAnalyzer", "Received data")

        val lastFiveSamples = samples.takeLast(5)

        val isCO2LevelHigh = lastFiveSamples.any { (it.co2Level.toDoubleOrNull() ?: 0.0) > 1000.0 }

        if (isCO2LevelHigh) {
            ApiClient.controlLed("warning_on")
            Log.w("SensorDataAnalyzer", "Detected high CO2 level!")
        } else {
            ApiClient.controlLed("warning_off")
            Log.d("SensorDataAnalyzer", "The co2 level in normal")
        }
    }
}