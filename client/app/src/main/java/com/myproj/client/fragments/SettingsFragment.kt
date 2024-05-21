package com.myproj.client.fragments

import android.content.Context
import android.graphics.Color
import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import com.myproj.client.R
import me.tankery.lib.circularseekbar.CircularSeekBar

class SettingsFragment : Fragment() {

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
            override fun onStopTrackingTouch(circularSeekBar: CircularSeekBar?) {}
        })

        return view
    }

    companion object {

        @JvmStatic
        fun newInstance() = SettingsFragment()
    }
}