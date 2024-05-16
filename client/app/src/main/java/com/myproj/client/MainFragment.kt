package com.myproj.client

import android.annotation.SuppressLint
import android.app.DatePickerDialog
import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.TextView
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.github.mikephil.charting.charts.LineChart
import com.myproj.client.databinding.ActivityMainBinding
import okhttp3.OkHttpClient
import okhttp3.ResponseBody
import org.json.JSONArray
import retrofit2.Call
import retrofit2.Callback
import retrofit2.Response
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory
import java.util.Calendar

class MainFragment : Fragment() {

    private var selectedDate = ""
    private lateinit var apiService: ApiService
    private val samples = mutableListOf<CO2Sample>()
    private lateinit var textView1: TextView
    val adapter = SampleAdapter()

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        // Inflate the layout for this fragment
        val view = inflater.inflate(R.layout.fragment_main, container, false)

        textView1 = view.findViewById(R.id.tvSelectedDate)

        val recView = view.findViewById<RecyclerView>(R.id.recView)
        recView.layoutManager = LinearLayoutManager(requireContext())
        recView.adapter = adapter

        ViewCompat.setOnApplyWindowInsetsListener(view.findViewById(R.id.main)) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }

        view.findViewById<Button>(R.id.btnSelectDate).setOnClickListener {
            showDatePickerDialog()
        }

        view.findViewById<Button>(R.id.btnAllMeasure).setOnClickListener {
            getSensorData("get_all", "")
        }

        view.findViewById<Button>(R.id.btnMeasureFromDate).setOnClickListener {
            getSensorData("get_after", selectedDate)
        }

        val retrofit = Retrofit.Builder().baseUrl("http://10.10.10.112:12345/")
            .addConverterFactory(GsonConverterFactory.create())
            .client(OkHttpClient.Builder().build()).build()

        apiService = retrofit.create(ApiService::class.java)
        getSensorData("get_all", "")

        return view
    }

    @SuppressLint("DefaultLocale")
    private fun showDatePickerDialog() {
        val calendar = Calendar.getInstance()
        val year = calendar.get(Calendar.YEAR)
        val month = calendar.get(Calendar.MONTH)
        val day = calendar.get(Calendar.DAY_OF_MONTH)

        val datePickerDialog = DatePickerDialog(
            requireContext(), { _, selectedYear, selectedMonth, selectedDay ->
                val formattedMonth = String.format("%02d", selectedMonth + 1)
                val formattedDay = String.format("%02d", selectedDay)
                selectedDate = "$selectedYear-$formattedMonth-$formattedDay"
                textView1.text = selectedDate
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
        @JvmStatic
        fun newInstance() = MainFragment()
    }
}
