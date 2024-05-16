package com.myproj.client

import android.annotation.SuppressLint
import android.graphics.Color
import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import com.github.mikephil.charting.charts.LineChart
import com.github.mikephil.charting.components.Description
import com.github.mikephil.charting.data.Entry
import com.github.mikephil.charting.data.LineData
import com.github.mikephil.charting.data.LineDataSet
import okhttp3.OkHttpClient
import okhttp3.ResponseBody
import org.json.JSONArray
import retrofit2.Call
import retrofit2.Callback
import retrofit2.Response
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory
import java.text.SimpleDateFormat
import java.util.Locale


class ChartFragment : Fragment() {

    private lateinit var lineChart: LineChart
    private lateinit var apiService: ApiService
    private val samples = mutableListOf<CO2Sample>()
    val adapter = SampleAdapter()

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        // Inflate the layout for this fragment
        val view = inflater.inflate(R.layout.fragment_chart, container, false)
        lineChart = view.findViewById(R.id.lineChart)

        ViewCompat.setOnApplyWindowInsetsListener(view.findViewById(R.id.chart_container)) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }
        val retrofit = Retrofit.Builder().baseUrl("http://10.10.10.112:12345/")
            .addConverterFactory(GsonConverterFactory.create())
            .client(OkHttpClient.Builder().build()).build()

        apiService = retrofit.create(ApiService::class.java)
        getSensorData("get_all", "")

        return view
    }

    private fun drawChart(samples: List<CO2Sample>) {

        val entries = samples.map { sample ->
            val timestamp = SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault()).parse(sample.datetime).time.toFloat()
            val co2Level = sample.co2Level.toFloat()
            Entry(timestamp, co2Level)
        }

        val lineDataSet = LineDataSet(entries, "CO2 Levels")
        lineDataSet.color = Color.RED
        lineDataSet.valueTextColor = Color.BLACK
        lineDataSet.valueTextSize = 7f
        lineDataSet.setDrawCircles(false)

        val lineData = LineData(lineDataSet)
        lineChart.data = lineData
        lineChart.description = Description().apply {
            text = "CO2 Measuring"
        }

        val xAxis = lineChart.xAxis
        xAxis.position = com.github.mikephil.charting.components.XAxis.XAxisPosition.BOTTOM
        xAxis.valueFormatter = DateAxisValueFormatter()
        xAxis.granularity = 3600000f
        xAxis.labelRotationAngle = 45f

        lineChart.axisRight.isEnabled = false
        val yAxisLeft = lineChart.axisLeft
        yAxisLeft.granularity = 10f

        lineChart.invalidate()
    }

    private fun getSensorData(command: String, param1: String) {
        val comm = Command(command, param1)
        apiService.getSensorData(comm).enqueue(object : Callback<ResponseBody> {
            @SuppressLint("NotifyDataSetChanged")
            override fun onResponse(call: Call<ResponseBody>, response: Response<ResponseBody>) {
                if (response.isSuccessful) {
                    response.body()?.let { body ->
                        val json = body.string()
                        adapter.clear()
                        if (json.isNotEmpty()) {
                            val jsonArray = JSONArray(json)
                            samples.clear()
                            for (i in 0 until jsonArray.length()) {
                                val jsonObject = jsonArray.getJSONObject(i)
                                val datetime = jsonObject.getString("datetime")
                                val co2Level = jsonObject.getString("CO2Level")
                                val sample = CO2Sample(datetime, co2Level)
                                samples.add(sample)
                            }

                            for (sample in samples) {
                                adapter.addSample(sample)
                            }
                            drawChart(samples)
                        } else {
                            //TODO: Обробка помилки
                        }
                    }
                } else {
                    //TODO: Обробка помилки
                }
            }

            override fun onFailure(call: Call<ResponseBody>, t: Throwable) {
                //TODO: Обробка помилки
            }
        })
    }



    companion object {
        fun newInstance() = ChartFragment()
    }
}