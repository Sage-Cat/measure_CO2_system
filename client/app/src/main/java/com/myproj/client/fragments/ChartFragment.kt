package com.myproj.client.fragments

import android.graphics.Color
import android.os.Bundle
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
import com.myproj.client.network.ApiService
import com.myproj.client.network.Command
import com.myproj.client.network.SensorDataCallback
import okhttp3.ResponseBody
import org.json.JSONArray
import retrofit2.Call
import retrofit2.Callback
import retrofit2.Response
import java.text.SimpleDateFormat
import java.util.Locale

class ChartFragment : Fragment() {

    private lateinit var lineChart: LineChart
    private val samplesIndoor = mutableListOf<CO2Sample>()
    private val samplesOutdoor = mutableListOf<CO2Sample>()
    private var requestsCompleted = 0

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {

        val view = inflater.inflate(R.layout.fragment_chart, container, false)
        lineChart = view.findViewById(R.id.lineChart)

        ViewCompat.setOnApplyWindowInsetsListener(view.findViewById(R.id.chart_container)) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }

        getSensorData("get_indoor", samplesIndoor)
        getSensorData("get_outdoor", samplesOutdoor)

        return view
    }

    private fun drawChart(samplesIndoor: List<CO2Sample>, samplesOutdoor: List<CO2Sample>) {
        val entriesIndoor = samplesIndoor.map { sample ->
            val timestamp = SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault()).parse(sample.datetime)?.time?.toFloat() ?: 0f
            val co2Level = sample.co2Level.toFloat()
            Entry(timestamp, co2Level)
        }

        val entriesOutdoor = samplesOutdoor.map { sample ->
            val timestamp = SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault()).parse(sample.datetime)?.time?.toFloat() ?: 0f
            val co2Level = sample.co2Level.toFloat()
            Entry(timestamp, co2Level)
        }

        val lineDataSetIndoor = LineDataSet(entriesIndoor, "Indoor CO2 Levels").apply {
            color = Color.RED
            valueTextColor = Color.BLACK
            valueTextSize = 7f
            setDrawCircles(false)
            setDrawValues(false) // прибрати позначення точок
        }

        val lineDataSetOutdoor = LineDataSet(entriesOutdoor, "Outdoor CO2 Levels").apply {
            color = Color.BLUE
            valueTextColor = Color.BLACK
            valueTextSize = 7f
            setDrawCircles(false)
            setDrawValues(false) // прибрати позначення точок
        }

        val lineData = LineData(lineDataSetIndoor, lineDataSetOutdoor)
        lineChart.data = lineData
        lineChart.description = Description().apply {
            text = "CO2 Measuring"
        }

        val xAxis = lineChart.xAxis
        xAxis.position = com.github.mikephil.charting.components.XAxis.XAxisPosition.BOTTOM
        xAxis.valueFormatter = DateAxisValueFormatter()
        xAxis.granularity = 3600000f
        xAxis.labelRotationAngle = 45f
        xAxis.yOffset = 10f // зсув підписів вгору

        lineChart.axisRight.isEnabled = false
        val yAxisLeft = lineChart.axisLeft
        yAxisLeft.granularity = 10f

        lineChart.invalidate()
    }

    private fun getSensorData(command: String, sampleList: MutableList<CO2Sample>) {
        ApiClient.getSensorData(command, "", object : SensorDataCallback {
            override fun onSuccess(samples: List<CO2Sample>) {
                sampleList.clear()
                sampleList.addAll(samples)
                requestsCompleted++
                if (requestsCompleted == 2) {
                    drawChart(samplesIndoor, samplesOutdoor)
                }
            }

            override fun onFailure(t: Throwable) {
                // Handle failure
            }
        })
    }


    companion object {
        fun newInstance() = ChartFragment()
    }
}
