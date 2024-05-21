package com.myproj.client.network

import android.util.Log
import com.myproj.client.CO2Sample
import com.myproj.client.SensorDataAnalyzer
import okhttp3.OkHttpClient
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory
import okhttp3.ResponseBody
import org.json.JSONArray
import retrofit2.Call
import retrofit2.Callback
import retrofit2.Response
import java.util.concurrent.Executors
import java.util.concurrent.ScheduledExecutorService
import java.util.concurrent.ScheduledFuture
import java.util.concurrent.TimeUnit

object ApiClient {
    private const val BASE_URL = "http://co2measure.local:12345/"
    private val executorService: ScheduledExecutorService = Executors.newSingleThreadScheduledExecutor()
    private var scheduledFuture: ScheduledFuture<*>? = null

    private val retrofit: Retrofit by lazy {
        Retrofit.Builder()
            .baseUrl(BASE_URL)
            .addConverterFactory(GsonConverterFactory.create())
            .client(OkHttpClient.Builder().build())
            .build()
    }

    private val apiService: ApiService by lazy {
        retrofit.create(ApiService::class.java)
    }

    fun startPolling(command: String, param1: String, interval: Long, callback: SensorDataCallback) {
        stopPolling()  // Зупиняємо будь-яке існуюче опитування
        scheduledFuture = executorService.scheduleWithFixedDelay({
            getSensorData(command, param1, callback)
        }, 0, interval, TimeUnit.SECONDS)
    }

    fun stopPolling() {
        scheduledFuture?.cancel(true)
        scheduledFuture = null
    }

    fun getSensorData(command: String, param1: String, callback: SensorDataCallback) {
        val comm = Command(command, param1)
        Log.d("ApiClient", "Sending request: $command with param: $param1")
        apiService.getSensorData(comm).enqueue(object : Callback<ResponseBody> {
            override fun onResponse(call: Call<ResponseBody>, response: Response<ResponseBody>) {
                if (response.isSuccessful) {
                    response.body()?.let { body ->
                        val json = body.string()
                        if (json.isNotEmpty()) {
                            val jsonArray = JSONArray(json)
                            val samples = mutableListOf<CO2Sample>()
                            for (i in 0 until jsonArray.length()) {
                                val jsonObject = jsonArray.getJSONObject(i)
                                val datetime = jsonObject.getString("datetime")
                                val co2Level = jsonObject.getString("CO2Level")
                                val sample = CO2Sample(datetime, co2Level)
                                samples.add(sample)
                            }
                            if (command != "get_outdoor") {
                                val analyzer = SensorDataAnalyzer()
                                analyzer.onDataReceived(samples)
                            }
                            callback.onSuccess(samples)
                        } else {
                            callback.onFailure(Throwable("Empty data"))
                        }
                    }
                } else {
                    callback.onFailure(Throwable("Unsuccessful response"))
                }
            }
            override fun onFailure(call: Call<ResponseBody>, t: Throwable) {
                callback.onFailure(t)
            }
        })
    }

    fun controlLed(command: String) {
        val comm = Command(command, "")
        Log.d("ApiClient", "Sending request to $command the LED")

        apiService.controlLed(comm).enqueue(object : Callback<ResponseBody> {
            override fun onResponse(call: Call<ResponseBody>, response: Response<ResponseBody>) {
                if (response.isSuccessful) {
                    Log.d("ApiClient", "LED control command successful")
                } else {
                    val errorBody = response.errorBody()?.string()
                    Log.e("ApiClient", "LED control failed: $errorBody")
                }
            }
            override fun onFailure(call: Call<ResponseBody>, t: Throwable) {
                Log.e("ApiClient", "LED control request failed", t)
            }
        })
    }
}
