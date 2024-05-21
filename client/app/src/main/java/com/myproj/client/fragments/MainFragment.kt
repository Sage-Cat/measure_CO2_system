package com.myproj.client.fragments

import android.annotation.SuppressLint
import android.app.DatePickerDialog
import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.myproj.client.CO2Sample
import com.myproj.client.R
import com.myproj.client.SampleAdapter
import com.myproj.client.network.ApiClient
import com.myproj.client.network.SensorDataCallback
import java.util.Calendar

class MainFragment : Fragment() {

    private var selectedDate = ""
    private val samples = mutableListOf<CO2Sample>()
    val adapter = SampleAdapter()

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        // Inflate the layout for this fragment
        val view = inflater.inflate(R.layout.fragment_main, container, false)

        val recView = view.findViewById<RecyclerView>(R.id.recView)
        recView.layoutManager = LinearLayoutManager(requireContext())
        recView.adapter = adapter

        ViewCompat.setOnApplyWindowInsetsListener(view.findViewById(R.id.main)) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }

        view.findViewById<Button>(R.id.btnAllMeasure).setOnClickListener {
            getSensorData("get_indoor", "")
        }

        view.findViewById<Button>(R.id.btnMeasureFromDate).setOnClickListener {
            showDatePickerDialog { selectedDate ->
                if (selectedDate.isNotEmpty()) {
                    getSensorData("get_indoor_after", selectedDate)
                }
            }
        }

        getSensorData("get_indoor", "")

        return view
    }

    @SuppressLint("DefaultLocale")
    private fun showDatePickerDialog(onDateSelected: (String) -> Unit) {
        val calendar = Calendar.getInstance()
        val year = calendar.get(Calendar.YEAR)
        val month = calendar.get(Calendar.MONTH)
        val day = calendar.get(Calendar.DAY_OF_MONTH)

        val datePickerDialog = DatePickerDialog(
            requireContext(), { _, selectedYear, selectedMonth, selectedDay ->
                val formattedMonth = String.format("%02d", selectedMonth + 1)
                val formattedDay = String.format("%02d", selectedDay)
                val selectedDate = "$selectedYear-$formattedMonth-$formattedDay"
                onDateSelected(selectedDate)
            }, year, month, day
        )
        datePickerDialog.show()
    }

    private fun getSensorData(command: String, param1: String) {
        ApiClient.getSensorData(command, param1, object : SensorDataCallback {
            override fun onSuccess(samples: List<CO2Sample>) {
                this@MainFragment.samples.clear()
                this@MainFragment.samples.addAll(samples)
                adapter.clear()
                for (sample in samples) {
                    adapter.addSample(sample)
                }
            }
            override fun onFailure(t: Throwable) {
                //TODO: Handle failure
            }
        })
    }

    companion object {
        @JvmStatic
        fun newInstance() = MainFragment()
    }
}
