package com.myproj.client

import org.junit.Test

import com.google.gson.Gson
import com.myproj.client.network.Command
import org.junit.Assert.assertEquals

/**
 * Example local unit test, which will execute on the development machine (host).
 *
 * See [testing documentation](http://d.android.com/tools/testing).
 */
class ExampleUnitTest {
    @Test
    fun commandUsesServerParamsArray() {
        val json = Gson().toJson(Command("get_indoor_after", listOf("2024-05-01 12:00:00")))
        assertEquals(
            "{\"cmd\":\"get_indoor_after\",\"params\":[\"2024-05-01 12:00:00\"]}",
            json
        )
    }
}
