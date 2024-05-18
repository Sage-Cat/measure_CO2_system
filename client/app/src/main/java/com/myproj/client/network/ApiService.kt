package com.myproj.client.network

import okhttp3.ResponseBody
import retrofit2.Call
import retrofit2.http.Body
import retrofit2.http.Headers
import retrofit2.http.POST

data class Command(val cmd: String, val param1: String)

interface ApiService {
    @Headers("Content-Type: application/json")
    @POST("/api/co2sensor")
    fun getSensorData(@Body command: Command): Call<ResponseBody>
}