package com.myproj.client.fragments

import android.graphics.Color
import android.os.Bundle
import android.util.Log
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import com.github.mikephil.charting.charts.LineChart
import com.github.mikephil.charting.components.Description
import com.github.mikephil.charting.data.Entry
import com.github.mikephil.charting.data.LineData
import com.github.mikephil.charting.data.LineDataSet
import com.myproj.client.CO2Sample
import com.myproj.client.DateAxisValueFormatter
import com.myproj.client.R
import com.myproj.client.network.ApiClient
import com.myproj.client.network.SensorDataCallback
import java.text.SimpleDateFormat
import java.util.Locale

class ChartFragment : Fragment() {

    private lateinit var lineChart1: LineChart
    private lateinit var lineChart2: LineChart
    private val samplesIndoor = mutableListOf<CO2Sample>()
    private val samplesOutdoor = mutableListOf<CO2Sample>()
    private val samplesForecast = mutableListOf<CO2Sample>()
    private var requestsCompleted = 0

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {

        val view = inflater.inflate(R.layout.fragment_chart, container, false)
        lineChart1 = view.findViewById(R.id.lineChart1)
        lineChart2 = view.findViewById(R.id.lineChart2)

        ViewCompat.setOnApplyWindowInsetsListener(view.findViewById(R.id.chart_container)) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }

        getSensorData("get_indoor", samplesIndoor)
        getSensorData("get_outdoor", samplesOutdoor)
        getSensorData("get_forecast", samplesForecast)

        return view
    }

    private fun getSensorData(command: String, sampleList: MutableList<CO2Sample>) {
        ApiClient.getSensorData(command, "", object : SensorDataCallback {
            override fun onSuccess(samples: List<CO2Sample>) {
                sampleList.clear()
                sampleList.addAll(samples)
                requestsCompleted++
                if (requestsCompleted == 3) {
                    samplesForecast.add(0, samplesIndoor.last())
                    drawCharts()
                }
            }

            override fun onFailure(t: Throwable) {
                Log.e("ChartFragment", "Failed to get data: $t")
            }
        })
    }


    private fun drawCharts() {
        drawChart(
            lineChart1,
            samplesIndoor,
            samplesOutdoor,
            "CO2 Measuring",
            "Indoor CO2 Levels",
            "Outdoor CO2 Levels",
            Color.RED,
            Color.BLUE
        )
        drawChart(
            lineChart2,
            samplesIndoor.takeLast(24),
            samplesForecast,
            "CO2 Forecasting",
            "Last 24 Indoor CO2 Levels",
            "Forecast CO2 Levels",
            Color.GREEN,
            Color.MAGENTA
        )
    }

    private fun drawChart(
        chart: LineChart,
        samples1: List<CO2Sample>,
        samples2: List<CO2Sample>,
        descriptionText: String,
        label1: String,
        label2: String,
        color1: Int,
        color2: Int
    ) {
        val entries1 = samples1.map { sample ->
            val timestamp = SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault()).parse(sample.datetime)?.time?.toFloat() ?: 0f
            Entry(timestamp, sample.co2Level.toFloat())
        }

        val entries2 = samples2.map { sample ->
            val timestamp = SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault()).parse(sample.datetime)?.time?.toFloat() ?: 0f
            Entry(timestamp, sample.co2Level.toFloat())
        }

        val lineDataSet1 = LineDataSet(entries1, label1).apply {
            color = color1
            valueTextColor = Color.BLACK
            valueTextSize = 7f
            setDrawCircles(false)
            setDrawValues(false)
        }

        val lineDataSet2 = LineDataSet(entries2, label2).apply {
            color = color2
            valueTextColor = Color.BLACK
            valueTextSize = 7f
            setDrawCircles(false)
            setDrawValues(false)
        }

        val lineData = LineData(lineDataSet1, lineDataSet2)
        chart.data = lineData
        chart.description = Description().apply {
            text = descriptionText
        }

        val xAxis = chart.xAxis
        xAxis.position = com.github.mikephil.charting.components.XAxis.XAxisPosition.BOTTOM
        xAxis.valueFormatter = DateAxisValueFormatter()
        xAxis.granularity = 3600000f
        xAxis.labelRotationAngle = 45f
        xAxis.yOffset = 10f

        chart.axisRight.isEnabled = false
        chart.axisLeft.granularity = 10f

        chart.invalidate()
    }


    companion object {
        fun newInstance() = ChartFragment()
    }
}
