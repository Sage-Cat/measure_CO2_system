package com.myproj.client.network

import android.util.Log
import com.myproj.client.CO2Sample
import okhttp3.OkHttpClient
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory
import okhttp3.ResponseBody
import org.json.JSONArray
import retrofit2.Call
import retrofit2.Callback
import retrofit2.Response

object ApiClient {
    private const val BASE_URL = "http://co2measure.local:12345/"

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
}