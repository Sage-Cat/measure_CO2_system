package com.myproj.client

import android.annotation.SuppressLint
import android.os.Bundle
import android.widget.TextView
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import okhttp3.OkHttpClient
import okhttp3.ResponseBody
import retrofit2.Call
import retrofit2.Callback
import retrofit2.Response
import java.util.Calendar
import android.app.DatePickerDialog
import android.graphics.Color
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.LinearLayoutManager
import com.github.mikephil.charting.charts.LineChart
import com.github.mikephil.charting.components.Description
import com.github.mikephil.charting.components.Legend
import com.github.mikephil.charting.components.XAxis
import com.github.mikephil.charting.data.Entry
import com.github.mikephil.charting.data.LineData
import com.github.mikephil.charting.data.LineDataSet
import com.github.mikephil.charting.formatter.IndexAxisValueFormatter
import com.myproj.client.databinding.ActivityMainBinding
import org.json.JSONArray
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

class MainActivity : AppCompatActivity() {

    private lateinit var bindingClass: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        bindingClass = ActivityMainBinding.inflate(layoutInflater)
        setContentView(bindingClass.root)
        replaceFragment(MainFragment())

        bindingClass.bottomNavigationView.setOnItemSelectedListener {
            when (it.itemId) {
                R.id.main -> replaceFragment(MainFragment())
                R.id.chart -> replaceFragment(ChartFragment())

                else -> {}
            }
            true
        }
    }

    private fun replaceFragment(fragment: Fragment) {

        val fragmentManager = supportFragmentManager
        val fragmentTransaction = fragmentManager.beginTransaction()
        fragmentTransaction.replace(R.id.frameLayout, fragment)
        fragmentTransaction.commit()
    }

    /*
     private lateinit var bindingClass: ActivityMainBinding
    private var selectedDate = ""
    private lateinit var apiService: ApiService
    private val samples = mutableListOf<CO2Sample>()
    private lateinit var textView1: TextView
    private lateinit var lineChart: LineChart
    val adapter = SampleAdapter()


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        bindingClass = ActivityMainBinding.inflate(layoutInflater)
        setContentView(bindingClass.root)
        textView1 = findViewById(R.id.textView)
        lineChart = findViewById<LineChart>(R.id.lineChart)

        bindingClass.recView.layoutManager = LinearLayoutManager(this)
        bindingClass.recView.adapter = adapter

        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main)) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }

        bindingClass.btnSelectDate.setOnClickListener {
            showDatePickerDialog()
        }

        bindingClass.btnAllMeasure.setOnClickListener {
            getSensorData("get_all", "")
        }

        bindingClass.btnMeasureFromDate.setOnClickListener {
            getSensorData("get_after", selectedDate)
        }

        val retrofit = Retrofit.Builder().baseUrl("http://10.10.10.112:12345/")
            .addConverterFactory(GsonConverterFactory.create())
            .client(OkHttpClient.Builder().build()).build()

        apiService = retrofit.create(ApiService::class.java)
        getSensorData("get_all", "")
        //drawChart(lineChart, samples)
    }

    private fun drawChart(samples: MutableList<CO2Sample>) {


        val samples2 = mutableListOf(
            CO2Sample("2024-05-16 10:00:00", "410"),
            CO2Sample("2024-05-16 11:00:00", "415"),
            CO2Sample("2024-05-16 12:00:00", "420")
        )

        // Convert CO2Sample data to chart entries
        val entries = samples.map { sample ->
            val timestamp = SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault()).parse(sample.datetime).time.toFloat()
            val co2Level = sample.co2Level.toFloat()
            Entry(timestamp, co2Level)
        }

        // Create a dataset from entries and give it a label
        val lineDataSet = LineDataSet(entries, "CO2 Levels")
        lineDataSet.color = Color.RED // Set the line color
        lineDataSet.valueTextColor = Color.BLACK // Set the value text color
        lineDataSet.valueTextSize = 7f // Set the size of the value text

        lineChart.legend.horizontalAlignment = Legend.LegendHorizontalAlignment.CENTER
        lineChart.legend.verticalAlignment = Legend.LegendVerticalAlignment.TOP
        lineDataSet.setDrawCircles(false)

        // Create LineData object that will hold the dataset
        val lineData = LineData(lineDataSet)
        lineChart.data = lineData

        // Configure X-axis
        val xAxis = lineChart.xAxis
        xAxis.position = com.github.mikephil.charting.components.XAxis.XAxisPosition.BOTTOM
        xAxis.valueFormatter = DateAxisValueFormatter()
        xAxis.granularity = 3600000f // one hour in milliseconds, adjust as necessary
        xAxis.labelRotationAngle = 45f

        // Configure Y-axis, if needed
        lineChart.axisRight.isEnabled = false // Disable the right Y-axis
        val yAxisLeft = lineChart.axisLeft
        yAxisLeft.granularity = 10f // Set granularity of the Y-axis, adjust as necessary

        // Refresh the chart
        lineChart.invalidate()
    }

    @SuppressLint("DefaultLocale")
    private fun showDatePickerDialog() {
        val calendar = Calendar.getInstance()
        val year = calendar.get(Calendar.YEAR)
        val month = calendar.get(Calendar.MONTH)
        val day = calendar.get(Calendar.DAY_OF_MONTH)

        val datePickerDialog = DatePickerDialog(
            this, { _, selectedYear, selectedMonth, selectedDay ->
                val formattedMonth = String.format("%02d", selectedMonth + 1)
                val formattedDay = String.format("%02d", selectedDay)
                selectedDate = "$selectedYear-$formattedMonth-$formattedDay"
                bindingClass.tvSelectedDate.text = selectedDate
            }, year, month, day
        )
        datePickerDialog.show()
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
    }*/

}
