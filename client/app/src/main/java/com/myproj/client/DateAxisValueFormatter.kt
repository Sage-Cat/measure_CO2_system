package com.myproj.client

import com.github.mikephil.charting.formatter.ValueFormatter
import java.text.SimpleDateFormat
import java.util.*

class DateAxisValueFormatter : ValueFormatter() {
    private val dateFormatter = SimpleDateFormat("MM/dd'\n'HH:mm", Locale.getDefault())

    override fun getFormattedValue(value: Float): String {
        return dateFormatter.format(Date(value.toLong()))
    }
}