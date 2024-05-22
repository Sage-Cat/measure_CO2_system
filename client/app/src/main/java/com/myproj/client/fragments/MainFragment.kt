package com.myproj.client.fragments

import android.content.Context
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import androidx.lifecycle.Observer
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.myproj.client.R
import com.myproj.client.SampleAdapter
import com.myproj.client.network.ApiClient
import com.myproj.client.network.DataRepository

class MainFragment : Fragment() {

    private lateinit var recyclerView: RecyclerView
    private lateinit var adapter: SampleAdapter
    private val dataRepository: DataRepository by viewModels()

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val view = inflater.inflate(R.layout.fragment_main, container, false)
        recyclerView = view.findViewById(R.id.recView)
        recyclerView.layoutManager = LinearLayoutManager(context)
        adapter = SampleAdapter()
        recyclerView.adapter = adapter
        return view
    }

    override fun onResume() {
        super.onResume()
        val sharedPrefs = requireActivity().getSharedPreferences("AppPreferences", Context.MODE_PRIVATE)
        val lastProgress = sharedPrefs.getFloat("LastProgress", 0f)

        dataRepository.sensorData.observe(viewLifecycleOwner) { sensorData ->
            if (sensorData.isNotEmpty()) {
                val currentCO2Level = sensorData.last().co2Level.toInt()
                val command = if (currentCO2Level >= lastProgress) "warning_on" else "warning_off"
                ApiClient.controlLed(command)
            }
        }
    }
    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        dataRepository.sensorData.observe(viewLifecycleOwner, Observer { samples ->
            Log.d("MainFragment", "Received samples: ${samples.size}")
            adapter.updateSamples(samples)
        })
    }
}
