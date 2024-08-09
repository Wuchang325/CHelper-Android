package yancey.chelper.android.about.activity;

import android.os.Bundle;
import android.widget.TextView;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import java.io.IOException;

import yancey.chelper.R;
import yancey.chelper.android.common.util.AssetsUtil;

/**
 * 软件的关于界面
 */
public class AboutActivity extends AppCompatActivity {

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.layout_about);
        TextView tv_about = findViewById(R.id.about);
        TextView tv_updateNote = findViewById(R.id.update_note);
        TextView tv_permissions = findViewById(R.id.permissions);
        TextView tv_dependencies = findViewById(R.id.dependencies);
        TextView tv_thanks = findViewById(R.id.thanks);
        // 从软件的内置资源读取内容并显示出来
        try {
            tv_about.setText(AssetsUtil.readStringFromAssets(this, "about/about.txt"));
            tv_updateNote.setText(AssetsUtil.readStringFromAssets(this, "about/update.txt"));
            tv_permissions.setText(AssetsUtil.readStringFromAssets(this, "about/permissions.txt"));
            tv_dependencies.setText(AssetsUtil.readStringFromAssets(this, "about/dependencies.txt"));
            tv_thanks.setText(AssetsUtil.readStringFromAssets(this, "about/thanks.txt"));
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

}
