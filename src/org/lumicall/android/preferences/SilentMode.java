package org.lumicall.android.preferences;

import android.app.Activity;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.view.View;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Toast;

import org.lumicall.android.R;
import org.sipdroid.sipua.ui.Receiver;
import org.sipdroid.sipua.ui.Settings;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;

/**
 * Created by urvikagola on 29/1/17.
 */

public class SilentMode extends Activity {
    EditText hhStart,mmStart,hhEnd,mmEnd;
    CheckBox mon, tue, wed, thur, fri, sat, sun;
    public static int activitycount=0;
    public Date date;
    public Date dateCompareOne;
    public Date dateCompareTwo;
    public SimpleDateFormat inputParser;
    public boolean checkboxflag;
    public static boolean ReturnSetHandler;
    static SilentMode sv;

    SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(Receiver.mContext);
    SharedPreferences.Editor editor = settings.edit();
    Boolean check_mon, check_tue, check_wed, check_thur, check_fri, check_sat, check_sun;
    String hourStart, minStart, hourEnd, minEnd;

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.silent_mode);
        activitycount=1;
        sv=this;

        hhStart = (EditText)findViewById(R.id.hour_start);
        mmStart = (EditText)findViewById(R.id.min_start);
        hhEnd = (EditText)findViewById(R.id.hour_end);
        mmEnd = (EditText)findViewById(R.id.min_end);
        mon = (CheckBox)findViewById(R.id.checkbox_mon);
        tue = (CheckBox)findViewById(R.id.checkbox_tue);
        wed = (CheckBox)findViewById(R.id.checkbox_wed);
        thur= (CheckBox)findViewById(R.id.checkbox_thur);
        fri = (CheckBox)findViewById(R.id.checkbox_fri);
        sat = (CheckBox)findViewById(R.id.checkbox_sat);
        sun = (CheckBox)findViewById(R.id.checkbox_sun);
        commitCheckBoxValues();
        commitEditTextValues();
    }

    public static SilentMode getInstance()         //To access the functions of this Activity in other Java files.
    {
        return  sv;
    }

    public void commitCheckBoxValues()
    {
        mon.setChecked(settings.getBoolean("checkbox_mon", false));
        tue.setChecked(settings.getBoolean("checkbox_tue", false));
        wed.setChecked(settings.getBoolean("checkbox_wed", false));
        thur.setChecked(settings.getBoolean("checkbox_thur", false));
        fri.setChecked(settings.getBoolean("checkbox_fri", false));
        sat.setChecked(settings.getBoolean("checkbox_sat", false));
        sun.setChecked(settings.getBoolean("checkbox_sun", false));

        mon.setOnClickListener(new View.OnClickListener(){
            public void onClick(View v){
                boolean checkBoxValue = ((CheckBox) v).isChecked();
                editor.putBoolean("checkbox_mon", checkBoxValue);
                editor.commit();
            }
        });

        tue.setOnClickListener(new View.OnClickListener(){
            public void onClick(View v){
                boolean checkBoxValue = ((CheckBox) v).isChecked();
                editor.putBoolean("checkbox_tue", checkBoxValue);
                editor.commit();
            }
        });

        wed.setOnClickListener(new View.OnClickListener(){
            public void onClick(View v){
                boolean checkBoxValue = ((CheckBox) v).isChecked();
                editor.putBoolean("checkbox_wed", checkBoxValue);
                editor.commit();
            }
        });

        thur.setOnClickListener(new View.OnClickListener(){
            public void onClick(View v){
                boolean checkBoxValue = ((CheckBox) v).isChecked();
                editor.putBoolean("checkbox_thur", checkBoxValue);
                editor.commit();
            }
        });

        fri.setOnClickListener(new View.OnClickListener(){
            public void onClick(View v){
                boolean checkBoxValue = ((CheckBox) v).isChecked();
                editor.putBoolean("checkbox_fri", checkBoxValue);
                editor.commit();
            }
        });

        sat.setOnClickListener(new View.OnClickListener(){
            public void onClick(View v){
                boolean checkBoxValue = ((CheckBox) v).isChecked();
                editor.putBoolean("checkbox_sat", checkBoxValue);
                editor.commit();
            }
        });

        sun.setOnClickListener(new View.OnClickListener(){
            public void onClick(View v){
                boolean checkBoxValue = ((CheckBox) v).isChecked();
                editor.putBoolean("checkbox_sun", checkBoxValue);
                editor.commit();
            }
        });
    }
    public void commitEditTextValues()
    {
        hhStart.setText(settings.getString("hourStart", ""));
        mmStart.setText(settings.getString("minStart", ""));
        hhEnd.setText(settings.getString("hourEnd", ""));
        mmEnd.setText(settings.getString("minEnd", ""));
    }
    public void getCheckBoxValue()
    {
        check_mon = settings.getBoolean("checkbox_mon", false);
        check_tue = settings.getBoolean("checkbox_tue", false);
        check_wed = settings.getBoolean("checkbox_wed", false);
        check_thur = settings.getBoolean("checkbox_thur", false);
        check_fri = settings.getBoolean("checkbox_fri", false);
        check_sat = settings.getBoolean("checkbox_sat", false);
        check_sun = settings.getBoolean("checkbox_sun", false);
    }
    public void setEditTextValue()
    {
        hourStart = hhStart.getText().toString();
        hourEnd = hhEnd.getText().toString();
        minStart = mmStart.getText().toString();
        minEnd = mmEnd.getText().toString();

        if(hourStart.equals("") && hourEnd.equals("") && minStart.equals("") && minEnd.equals(""))
        {
            editor.putString("hourStart", "");
            editor.putString("hourEnd", "");
            editor.putString("minStart", "");
            editor.putString("minEnd", "");
        }
        else
        {
            editor.putString("hourStart" , hourStart);
            editor.putString("hourEnd" , hourEnd);
            editor.putString("minStart" , minStart);
            editor.putString("minEnd" , minEnd);
        }
        editor.commit();
    }

    public boolean checkDay()
    {
        Calendar calendar = Calendar.getInstance();
        int day = calendar.get(Calendar.DAY_OF_WEEK);
        //int SUNDAY=1, MONDAY=2..
        boolean dayflag=false;
        if(check_mon && day == 2){
            dayflag=true;
        }
        else if(check_tue && day == 3){
            dayflag=true;
        }
        else if(check_wed && day == 4){
            dayflag = true;
        }
        else if(check_thur && day == 5){
            dayflag = true;
        }
        else if(check_fri && day == 6){
            dayflag = true;
        }
        else if(check_sat && day == 7){
            dayflag = true;
        }
        else if(check_sun && day == 1){
            dayflag = true;
          }
        return dayflag;
    }

    public boolean compareDates()
    {
        if( hhStart.getText().toString().equals("") &&
                mmStart.getText().toString().equals("") &&
                hhEnd.getText().toString().equals("") &&
                mmEnd.getText().toString().equals(""))
        //If no time constraints are set by user, the silent mode will be enabled for the entire day
        {
            return true;
        }
        else
        {
            String compareStringOne = hourStart + ":" + minStart;
            String compareStringTwo = hourEnd + ":" + minEnd;
            String inputFormat = "HH:mm";

            inputParser = new SimpleDateFormat(inputFormat, Locale.ENGLISH);
            Calendar now = Calendar.getInstance();
            int hour = now.get(Calendar.HOUR_OF_DAY);
            int minute = now.get(Calendar.MINUTE);

            date = parseDate(hour + ":" + minute);
            dateCompareOne = parseDate(compareStringOne);
            dateCompareTwo = parseDate(compareStringTwo);

            if (dateCompareOne.before(date) && dateCompareTwo.after(date))
            {
                return  true;
            }
            else
            {
                return false;
            }
        }
    }

    public Date parseDate(String date)
    {
        try {
            return inputParser.parse(date);
        } catch (java.text.ParseException e)
        {
            return new Date(0);
        }
    }

    public boolean isEmpty()
    {
        //Silent mode should be enabled without any constraints if Checkboxes and Time Text Fields are empty
        if((checkboxflag) && (!check_mon) &&
                (!check_tue) && (!check_wed) &&
                (!check_thur) && (!check_fri) &&
                (!check_sat) && (!check_sun) &&
                (hourStart.equals("")) && (minStart.equals("")) &&
                (hourEnd.equals("")) && (minEnd.equals("")))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    public void setHandler(View target)
    {
        Toast.makeText(getApplicationContext(), "Settings Saved", Toast.LENGTH_SHORT).show();
    }

    public boolean checkSilentMode()
    {
        ReturnSetHandler = false;
        getCheckBoxValue();
        setEditTextValue();

        checkboxflag=PreferenceManager.getDefaultSharedPreferences(Receiver.mContext).getBoolean(Settings.PREF_SILENT_MODE, Settings.DEFAULT_SILENT_MODE);
        if(isEmpty())
        {
            ReturnSetHandler = true;
        }

        else if(checkDay())
        {
            //If current day is checked by the user , compare Time
            if(compareDates())//If the current time falls in user time constraint
            {
                ReturnSetHandler = true;
            }
            else        //Days match, timings don't.
            {
                ReturnSetHandler = false;
            }
        }
        return ReturnSetHandler;
    }
}