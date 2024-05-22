package com.myproj.client.fragments

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.myproj.client.CO2Sample
import com.myproj.client.R
import com.myproj.client.SampleAdapter
import com.myproj.client.network.SensorDataViewModel

class MainFragment : Fragment() {

    private val viewModel: SensorDataViewModel by viewModels()
    private lateinit var recyclerView: RecyclerView
    private lateinit var adapter: SampleAdapter

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val view = inflater.inflate(R.layout.fragment_main, container, false)
        recyclerView = view.findViewById(R.id.recView)
        recyclerView.layoutManager = LinearLayoutManager(context)
        adapter = SampleAdapter()
        recyclerView.adapter = adapter

        val btnAllMeasure = view.findViewById<Button>(R.id.btnAllMeasure)
        val btnMeasureFromDate = view.findViewById<Button>(R.id.btnMeasureFromDate)

        btnMeasureFromDate.setOnClickListener{

        }


        return view
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        viewModel.sensorData.observe(viewLifecycleOwner) { samples ->
            Log.d("MainFragment", "Received samples: ${samples.size}")
            adapter.updateSamples(samples)
        }
    }
}