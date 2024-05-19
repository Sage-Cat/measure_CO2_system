package com.myproj.client.network

import com.myproj.client.CO2Sample

interface SensorDataCallback {
    fun onSuccess(samples: List<CO2Sample>)
    fun onFailure(t: Throwable)
}