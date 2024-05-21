package com.myproj.client.network

import android.os.Handler
import android.os.Looper
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.myproj.client.CO2Sample

class SensorDataViewModel : ViewModel() {

    private val _sensorData = MutableLiveData<List<CO2Sample>>()
    val sensorData: LiveData<List<CO2Sample>> get() = _sensorData

    private val callback = object : SensorDataCallback {
        override fun onSuccess(samples: List<CO2Sample>) {
            Handler(Looper.getMainLooper()).post {
                _sensorData.value = samples
            }
        }

        override fun onFailure(t: Throwable) {
            // Обробка помилок
        }
    }
    init {
        startPolling("get_indoor", "", 30)
    }

    private fun startPolling(command: String, param1: String, interval: Long) {
        ApiClient.startPolling(command, param1, interval, callback)
    }

    override fun onCleared() {
        super.onCleared()
        stopPolling()
    }

    private fun stopPolling() {
        ApiClient.stopPolling()
    }
}