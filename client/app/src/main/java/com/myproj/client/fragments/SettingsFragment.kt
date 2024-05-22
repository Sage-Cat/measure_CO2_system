package com.myproj.client.fragments

import android.content.Context
import android.graphics.Color
import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.fragment.app.viewModels
import androidx.lifecycle.Observer
import com.myproj.client.R
import com.myproj.client.network.ApiClient
import com.myproj.client.network.DataRepository
import me.tankery.lib.circularseekbar.CircularSeekBar

class SettingsFragment : Fragment() {

    private val dataRepository: DataRepository by viewModels()

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        // Inflate the layout for this fragment
        val view = inflater.inflate(R.layout.fragment_settings, container, false)

        val circularSeekBar = view.findViewById<CircularSeekBar>(R.id.circularSeekBar)
        val progressTextView = view.findViewById<TextView>(R.id.progressTextView)

        val sharedPrefs = requireActivity().getSharedPreferences("AppPreferences", Context.MODE_PRIVATE)
        val lastProgress = sharedPrefs.getFloat("LastProgress", 0f)

        circularSeekBar.progress = lastProgress
        onSeekBarChange(circularSeekBar.progress.toInt())
        progressTextView.text = Math.round(lastProgress).toString()

        circularSeekBar.setOnSeekBarChangeListener(object : CircularSeekBar.OnCircularSeekBarChangeListener {
            override fun onProgressChanged(circularSeekBar: CircularSeekBar?, progress: Float, fromUser: Boolean) {
                if (circularSeekBar != null) {
                    progressTextView.text = Math.round(circularSeekBar.progress).toString()
                    with(requireActivity().getSharedPreferences("AppPreferences", Context.MODE_PRIVATE).edit()) {
                        putFloat("LastProgress", circularSeekBar.progress)
                        apply()
                    }
                }
            }

            override fun onStartTrackingTouch(circularSeekBar: CircularSeekBar?) {}
            override fun onStopTrackingTouch(circularSeekBar: CircularSeekBar?) {
                if (circularSeekBar != null) {
                    onSeekBarChange(circularSeekBar.progress.toInt())
                }
            }
        })

        return view
    }

    private fun onSeekBarChange(newValue: Int) {
        dataRepository.sensorData.observe(viewLifecycleOwner, Observer { sensorData ->
            var command = "warning_off"
            if (sensorData.isNotEmpty() && sensorData.last().co2Level.toInt() >= newValue) {
               command = "warning_on"
            }
            ApiClient.controlLed(command)
        })
    }

    companion object {

        @JvmStatic
        fun newInstance() = SettingsFragment()
    }
}