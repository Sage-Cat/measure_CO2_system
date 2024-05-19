package com.myproj.client

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.myproj.client.databinding.SampleItemBinding

class SampleAdapter : RecyclerView.Adapter<SampleAdapter.SampleHolder>() {
    private val sampleList = ArrayList<CO2Sample>()
    class SampleHolder(item: View): RecyclerView.ViewHolder(item) {
        val binding = SampleItemBinding.bind(item)
        fun bind(cO2Sample: CO2Sample){
            binding.date.text = cO2Sample.datetime
            binding.value.text = cO2Sample.co2Level
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): SampleHolder {
        val view = LayoutInflater.from(parent.context).inflate(R.layout.sample_item, parent, false)
        return SampleHolder(view)
    }

    override fun getItemCount(): Int {
        return sampleList.size
    }

    override fun onBindViewHolder(holder: SampleHolder, position: Int) {
        holder.bind(sampleList[position])
    }

    fun clear(){
        sampleList.clear()
    }

    fun addSample(sample: CO2Sample){

        sampleList.add(sample)
        notifyDataSetChanged()
    }
}