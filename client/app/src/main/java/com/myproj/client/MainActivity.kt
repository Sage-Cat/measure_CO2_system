package com.myproj.client


import android.os.Bundle
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.Fragment
import com.myproj.client.databinding.ActivityMainBinding
import com.myproj.client.fragments.ChartFragment
import com.myproj.client.fragments.MainFragment
import com.myproj.client.fragments.SettingsFragment

class MainActivity : AppCompatActivity() {

    private lateinit var bindingClass: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        bindingClass = ActivityMainBinding.inflate(layoutInflater)
        setContentView(bindingClass.root)
        replaceFragment(MainFragment())

        bindingClass.bottomNavigationView.setOnItemSelectedListener {
            when (it.itemId) {
                R.id.main -> replaceFragment(MainFragment())
                R.id.chart -> replaceFragment(ChartFragment())
                //R.id.settings -> replaceFragment(SettingsFragment())

                else -> {}
            }
            true
        }
    }

    private fun replaceFragment(fragment: Fragment) {

        val fragmentManager = supportFragmentManager
        val fragmentTransaction = fragmentManager.beginTransaction()
        fragmentTransaction.replace(R.id.frameLayout, fragment)
        fragmentTransaction.commit()
    }
}
