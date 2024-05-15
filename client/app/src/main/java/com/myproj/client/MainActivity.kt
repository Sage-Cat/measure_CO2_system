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
import androidx.recyclerview.widget.LinearLayoutManager
import com.myproj.client.databinding.ActivityMainBinding
import org.json.JSONArray
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory

class MainActivity : AppCompatActivity() {

    private lateinit var bindingClass: ActivityMainBinding
    private var selectedDate = ""
    private lateinit var apiService: ApiService
    private val samples = mutableListOf<CO2Sample>()
    private lateinit var textView1: TextView
    val adapter = SampleAdapter()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        bindingClass = ActivityMainBinding.inflate(layoutInflater)
        setContentView(bindingClass.root)
        textView1 = findViewById(R.id.textView)

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
                                val co2Level = jsonObject.getString("CO2Level") + "ppm"
                                val sample = CO2Sample(datetime, co2Level)
                                samples.add(sample)
                            }

                            for (sample in samples) {
                                adapter.addSample(sample)
                            }
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

}