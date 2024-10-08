package yancey.chelper.android.welcome.activity;

import android.content.Intent;
import android.os.Bundle;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.ImageView;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.lzf.easyfloat.EasyFloat;
import com.lzf.easyfloat.enums.ShowPattern;
import com.lzf.easyfloat.permission.PermissionUtils;

import yancey.chelper.R;
import yancey.chelper.android.about.activity.AboutActivity;
import yancey.chelper.android.common.dialog.IsConfirmDialog;
import yancey.chelper.android.common.util.ToastUtil;
import yancey.chelper.android.completion.activity.CompletionActivity;
import yancey.chelper.android.completion.activity.SettingsActivity;
import yancey.chelper.android.enumeration.activity.EnumerationActivity;
import yancey.chelper.android.favorites.activity.FavoritesActivity;
import yancey.chelper.android.old2new.activity.Old2NewActivity;
import yancey.chelper.android.old2new.activity.Old2NewIMEGuideActivity;
import yancey.chelper.android.rawtext.activity.RawtextActivity;
import yancey.chelper.android.welcome.view.FloatingMainView;

/**
 * 欢迎界面 + 悬浮窗管理
 */
public class WelcomeActivity extends AppCompatActivity {

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_welcome);
        findViewById(R.id.btn_start_suggestion_app).setOnClickListener(v -> {
            if (isUsingFloatingWindow()) {
                ToastUtil.show(this, "你必须关闭悬浮窗模式才可以进入应用模式");
                return;
            }
            startActivity(new Intent(this, CompletionActivity.class));
        });
        findViewById(R.id.btn_start_enumeration_window).setOnClickListener(v -> {
            if (isUsingFloatingWindow()) {
                stopFloatingWindow();
            } else {
                startFloatingWindow(40);
            }
        });
        findViewById(R.id.btn_enumeration_settings).setOnClickListener(v -> startActivity(new Intent(this, SettingsActivity.class)));
        findViewById(R.id.btn_start_old2new_app).setOnClickListener(v -> startActivity(new Intent(this, Old2NewActivity.class)));
        findViewById(R.id.btn_start_old2new_ime).setOnClickListener(v -> startActivity(new Intent(this, Old2NewIMEGuideActivity.class)));
        findViewById(R.id.btn_raw_json_studio).setOnClickListener(v -> startActivity(new Intent(this, RawtextActivity.class)));
        findViewById(R.id.btn_enumeration).setOnClickListener(v -> startActivity(new Intent(this, EnumerationActivity.class)));
        findViewById(R.id.btn_favorite).setOnClickListener(v -> startActivity(new Intent(this, FavoritesActivity.class)));
        findViewById(R.id.btn_about).setOnClickListener(v -> startActivity(new Intent(this, AboutActivity.class)));
    }

    /**
     * 是否使用着悬浮穿
     *
     * @return 是否使用着悬浮穿
     */
    private boolean isUsingFloatingWindow() {
        return EasyFloat.getFloatView("icon_view") != null;
    }

    /**
     * 开启悬浮窗
     *
     * @param iconSize 图标大小
     */
    private void startFloatingWindow(int iconSize) {
        if (!PermissionUtils.checkPermission(this)) {
            new IsConfirmDialog(this, false)
                    .message("需要悬浮窗权限，请进入设置进行授权")
                    .onConfirm("打开设置", v -> PermissionUtils.requestPermission(this, isOpen -> {
                        if (isOpen) {
                            startFloatingWindow(iconSize);
                        } else {
                            ToastUtil.show(WelcomeActivity.this, "悬浮窗权限获取失败");
                        }
                    })).show();
            return;
        }
        int length = (int) TypedValue.applyDimension(
                TypedValue.COMPLEX_UNIT_DIP,
                iconSize,
                getResources().getDisplayMetrics()
        );
        FloatingMainView floatingMainView = new FloatingMainView(this, this::stopFloatingWindow, length);
        Runnable hide = () -> {
            EasyFloat.updateFloat("icon_view", (int) floatingMainView.getIconViewX(), (int) floatingMainView.getIconViewY());
            EasyFloat.hide("main_view");
            EasyFloat.clearFocus("main_view");
            floatingMainView.onPause();
        };
        floatingMainView.setOnIconClickListener(hide);
        ImageView iconView = new ImageView(this);
        iconView.setImageResource(R.drawable.pack_icon);
        iconView.setLayoutParams(new FrameLayout.LayoutParams(length, length, Gravity.START | Gravity.TOP));
        iconView.setOnClickListener(v -> {
            if (EasyFloat.getFloatView("main_view") == null) {
                EasyFloat.with(this)
                        .setLayout(floatingMainView)
                        .setTag("main_view")
                        .setShowPattern(ShowPattern.ALL_TIME)
                        .setDragEnable(false)
                        .setMatchParent(true, true)
                        .hasEditText(true)
                        .setAnimator(null)
                        .show();
                floatingMainView.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
                    @Override
                    public void onGlobalLayout() {
                        floatingMainView.getViewTreeObserver().removeOnGlobalLayoutListener(this);
                        EasyFloat.requestFocus("main_view");
                    }
                });
            } else {
                EasyFloat.show("main_view");
                EasyFloat.requestFocus("main_view");
            }
            WindowManager.LayoutParams layoutParam = EasyFloat.getLayoutParam("icon_view");
            if (layoutParam != null) {
                floatingMainView.setIconPosition(layoutParam.x, layoutParam.y);
            }
            floatingMainView.onResume();
        });
        EasyFloat.with(this)
                .setLayout(iconView)
                .setTag("icon_view")
                .setShowPattern(ShowPattern.ALL_TIME)
                .setDragEnable(true)
                .setGravity(Gravity.START | Gravity.TOP)
                .setAnimator(null)
                .show();
    }

    /**
     * 关闭悬浮窗
     */
    private void stopFloatingWindow() {
        FloatingMainView floatingMainView = (FloatingMainView) EasyFloat.getFloatView("main_view");
        if (floatingMainView != null) {
            floatingMainView.onPause();
            floatingMainView.onDestroy();
        }
        EasyFloat.dismiss("icon_view");
        EasyFloat.dismiss("main_view");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        stopFloatingWindow();
    }

}
