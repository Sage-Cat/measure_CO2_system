package com.myproj.client

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.myproj.client.databinding.SampleItemBinding

class SampleAdapter(private var sampleList: List<CO2Sample> = emptyList()) : RecyclerView.Adapter<SampleAdapter.SampleViewHolder>() {

    class SampleViewHolder(val binding: SampleItemBinding) : RecyclerView.ViewHolder(binding.root)

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): SampleViewHolder {
        val binding = SampleItemBinding.inflate(LayoutInflater.from(parent.context), parent, false)
        return SampleViewHolder(binding)
    }

    override fun onBindViewHolder(holder: SampleViewHolder, position: Int) {
        val sample = sampleList[position]
        holder.binding.date.text = sample.datetime
        holder.binding.value.text = sample.co2Level
    }

    override fun getItemCount(): Int = sampleList.size

    fun updateSamples(newSamples: List<CO2Sample>) {
        this.sampleList = newSamples.reversed()
        notifyDataSetChanged()
    }
}