#include<stdio.h>
#include<stdbool.h>
#include<iostream>
#include<graphics.h>
#include<mmsystem.h>
#include<conio.h>
#include<time.h>
#include<chrono>
#include<math.h>
#pragma comment( lib, "MSIMG32.LIB")
#pragma comment(lib,"winmm.lib")
#define width 1000
#define height 600
#define fps 25
using ull = unsigned long long;
bool cli_up;
bool cli_down;
enum MyEnum { F1, F2, count };
enum GameState { PLAYING, WIN, LOSE };
GameState gameState = PLAYING;
IMAGE winImg, loseImg;
int moving_x, moving_y;
bool if_moving = false;
int sign_index = -1;
int sun_value = 50;
IMAGE backgrand;
IMAGE backcard;
IMAGE card[count];
IMAGE moving_card[count][fps];
IMAGE zomb_dead[20];
int total_frames[count] = { 0 }; // 新增：存储每个植物类型的动画总帧数
struct mmp {
    int type;
    int fps_index;
    int hp;
    bool ate;//if正在被吃
    int sun_timer;
};
struct mmp map[9][3];
IMAGE mmp_eat[22];
const int SUN_COST[count] = { 100, 50 }; // F1=100, F2=50
struct sun {
    int x, y;
    int end_y;
    bool used;
    int fps_index;
    int timer;
    float x_off, y_off;//速度
};
struct sun sun_pool[10];
IMAGE sun_img[29];
struct zombie {
    int x, y;
    bool used;
    int fps_index;
    int speed;
    int row;
    int hp;
    bool dead;
    int dead_index;

    bool eating;
    int eating_index;
};
struct zombie zomb[10];
IMAGE zomb_img[22];

struct bullets {
    int x, y, row;
    int speed;
    bool used;
    bool blast;
    int fps_index;
};
struct bullets bts[9999];
IMAGE bts_img1;
IMAGE bts_img2[4];
bool is_empty(const char* name) {
    FILE* file;
    errno_t err = fopen_s(&file, name, "r");
    if (err != 0 || file == NULL) {
        return true;
    }
    fclose(file);
    return false;
}
void new_png(IMAGE* dstimg, int x, int y, IMAGE* srcimg) {
    HDC dstDC = GetImageHDC(dstimg);
    HDC srcDC = GetImageHDC(srcimg);
    int w = srcimg->getwidth();
    int h = srcimg->getheight();
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    AlphaBlend(dstDC, x, y, w, h, srcDC, 0, 0, w, h, bf);
}
void init() {
    loadimage(&backgrand, "C:/Users/amd/Desktop/植物大战僵尸/res/bg.jpg");
    loadimage(&backcard, "C:/Users/amd/Desktop/植物大战僵尸/res/bar4.png");
    initgraph(width, height, EW_SHOWCONSOLE);

    LOGFONT f;
    gettextstyle(&f);
    f.lfHeight = 30;
    f.lfWeight = 15;
    strcpy_s(f.lfFaceName, sizeof(f.lfFaceName), "Segoe UI Black");
    f.lfQuality = ANTIALIASED_QUALITY;
    settextstyle(&f);
    setbkmode(TRANSPARENT);
    setcolor(BLACK);

    char name[64];
    for (int i = 0; i < count; i++) {
        sprintf_s(name, sizeof(name), "C:/Users/amd/Desktop/植物大战僵尸/res/Cards/card_%d.png", i + 1);
        loadimage(&card[i], name);
        srand(time(NULL));
        // 加载植物动画帧并统计总帧数
        int loadedFrames = 0;
        for (int j = 0; j < fps; j++) {
            sprintf_s(name, sizeof(name), "C:/Users/amd/Desktop/植物大战僵尸/res/Cards/zhiwu/%d/%d.png", i, j + 1);
            if (!is_empty(name)) {
                loadimage(&moving_card[i][j], name);
                loadedFrames++;
            }
            else {
                break;
            }
        }
        total_frames[i] = loadedFrames; // 记录实际加载的帧数
    }
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 3; j++) {
            map[i][j].type = -1;
            map[i][j].fps_index = 0;
            map[i][j].sun_timer = 0;
            if (map[i][j].type != -1 && !map[i][j].ate) {
                map[i][j].hp = 100;
            }
        }
    }

    memset(sun_pool, 0, sizeof(sun_pool));
    for (int i = 0; i < 29; i++) {
        sprintf_s(name, sizeof(name), "C:/Users/amd/Desktop/植物大战僵尸/res/sunshine/%d.png", i + 1);
        loadimage(&sun_img[i], name);
    }

    memset(zomb, 0, sizeof(zomb));
    for (int i = 0; i < 22; i++) {
        sprintf_s(name, sizeof(name), "C:/Users/amd/Desktop/植物大战僵尸/res/zm/%d.png", i + 1);
        loadimage(&zomb_img[i], name);
    }

    memset(bts, 0, sizeof(bts));
    loadimage(&bts_img1, "C:/Users/amd/Desktop/植物大战僵尸/res/bullets/bullet_normal.png");

    loadimage(&bts_img2[3], "C:/Users/amd/Desktop/植物大战僵尸/res/bullets/bullet_blast.png");
    for (int i = 2; i <= 4; i++) {
        float k = 0.2 * i;
        loadimage(&bts_img2[i - 2], "C:/Users/amd/Desktop/植物大战僵尸/res/bullets/bullet_blast.png", bts_img2[3].getheight() * k, bts_img2[3].getwidth() * k, true);
    }

    for (int i = 0; i < 20; i++) {
        sprintf_s(name, sizeof(name), "C:/Users/amd/Desktop/植物大战僵尸/res/zm_dead/%d.png", i + 1);
        loadimage(&zomb_dead[i], name);
    }

    for (int i = 0; i < 22; i++) {
        sprintf_s(name, sizeof(name), "C:/Users/amd/Desktop/植物大战僵尸/res/zm_eat/%d.png", i + 1);
        loadimage(&mmp_eat[i], name);
    }
    loadimage(&winImg, "C:/Users/amd/Desktop/植物大战僵尸/res/gameWin.png");
    loadimage(&loseImg, "C:/Users/amd/Desktop/植物大战僵尸/res/gameFail.png");
}
void draw_zomb() {
    int sum_zomb = sizeof(zomb) / sizeof(zomb[0]);
    BeginBatchDraw();
    for (int i = 0; i < sum_zomb; i++) {
        if (zomb[i].used) {
            if (zomb[i].dead) {
                IMAGE* img = &zomb_dead[zomb[i].dead_index];
                new_png(NULL, zomb[i].x, zomb[i].y - img->getheight(), img);
            }
            else if (zomb[i].eating) {
                // 正在啃食，使用啃食动画帧
                IMAGE* img = &mmp_eat[zomb[i].eating_index % 21];
                new_png(NULL, zomb[i].x, zomb[i].y - img->getheight(), img);
            }
            else {
                // 正常行走状态
                IMAGE* img = &zomb_img[zomb[i].fps_index];
                new_png(NULL, zomb[i].x, zomb[i].y - img->getheight(), img);
            }
        }
    }
    EndBatchDraw();
}
void check_victory() {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start);

    if (elapsed.count() >= 180) { // 180秒后胜利
        gameState = WIN;
    }
}
void update() {
    BeginBatchDraw();
    putimage(0, 0, &backgrand);
    putimage(250, 0, &backcard);
    for (int i = 0; i < count; i++) {
        int x = 323 + i * 65;
        putimage(x, -8, &card[i]);
    }
    if (if_moving) {
        new_png(NULL, moving_x - 37, moving_y - 37, &moving_card[sign_index][0]);
    }
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 3; j++) {
            if (map[i][j].type >= 0 && map[i][j].fps_index < total_frames[map[i][j].type]) {
                int x = 250 + i * 83;
                int y = 175 + j * 103;
                new_png(NULL, x, y, &moving_card[map[i][j].type][map[i][j].fps_index]);
            }
        }
    }
    //阳光
    int max_num = sizeof(sun_pool) / sizeof(sun_pool[0]);
    for (int i = 0; i < max_num; i++) {
        if (sun_pool[i].used || sun_pool[i].y_off) {
            new_png(NULL, sun_pool[i].x, sun_pool[i].y, &sun_img[sun_pool[i].fps_index]);
        }
    }

    char sun_text[4];
    sprintf_s(sun_text, sizeof(sun_text), "%d", sun_value);
    outtextxy(276, 55, sun_text);

    draw_zomb();

    int count_b = sizeof(bts) / sizeof(bts[0]);
    for (int i = 0; i < count_b; i++) {
        if (bts[i].used) {
            if (bts[i].blast) {
                new_png(NULL, bts[i].x, bts[i].y, &bts_img2[bts[i].fps_index]);
            }
            else {
                new_png(NULL, bts[i].x, bts[i].y, &bts_img1);
            }
        }
    }
    if (gameState != PLAYING) {
        BeginBatchDraw();
        putimage(0, 0, gameState == WIN ? &winImg : &loseImg);
        EndBatchDraw();
        return;
    }
    check_victory();
    EndBatchDraw();
}
void sunshine_down() {
    int max_num = sizeof(sun_pool) / sizeof(sun_pool[0]);
    for (int i = 0; i < max_num; i++) {
        if (sun_pool[i].used) {
            if (sun_pool[i].timer == 0)
                sun_pool[i].y += 3;
            sun_pool[i].fps_index = (sun_pool[i].fps_index + 1) % 29;
            if (sun_pool[i].y >= sun_pool[i].end_y) {
                sun_pool[i].timer++;
                if (sun_pool[i].timer > 25) {
                    sun_pool[i].used = false;
                }
            }
        }
        //used这个逻辑
        else if (sun_pool[i].y_off) {
            sun_pool[i].x -= sun_pool[i].x_off;
            sun_pool[i].y -= sun_pool[i].y_off;
            if (sun_pool[i].y < 0 || sun_pool[i].x < 262) {
                sun_pool[i].y_off = 0, sun_pool[i].x_off = 0;
                sun_value += 25;
            }
        }
    }
}
void sunshine_ini() {
    //防止阳光产生过快
    static int count = 0, num = 100;
    count++;
    if (count >= num) {
        count = 0;
        num = 150 + rand() % 100;
        int pool_size = sizeof(sun_pool) / sizeof(sun_pool[0]);
        int i = 0;
        //使用过，向下滑动
        for (; i < pool_size && sun_pool[i].used; i++);
        if (i >= pool_size)    return;
        sun_pool[i].x = 330 + rand() % (900 - 330);
        sun_pool[i].end_y = 170 + rand() % (500 - 170);
        sun_pool[i].y = 170;
        sun_pool[i].used = true;
        sun_pool[i].timer = 0;
        sun_pool[i].x_off = 0;
        sun_pool[i].y_off = 0;
        std::cout << "Created sunshine at_" << sun_pool[i].x << "_" << sun_pool[i].y << std::endl;
    }
}
void crate_zomb() {
    static int count = 0, num = 300;
    count++;
    if (count >= num) {
        count = 0;
        num = 150 + rand() % 100;
        int i = 0;
        int zomb_size = sizeof(zomb) / sizeof(zomb[0]);
        for (; i < zomb_size; i++) {
            // 找到一个未使用的僵尸位置
            if (!zomb[i].used) {  // 修改后的条件
                zomb[i].used = true;
                zomb[i].x = 1000;
                zomb[i].row = 1 + rand() % 3;
                zomb[i].y = 175 + (zomb[i].row) * 100;
                zomb[i].speed = 1;
                zomb[i].hp = 200;
                zomb[i].dead = false;        // 添加初始化
                zomb[i].dead_index = 0;      // 添加初始化
                zomb[i].fps_index = 0;       // 添加初始化
                break;
            }
        }
    }
}
void update_zomb() {
    int sum_zmb = sizeof(zomb) / sizeof(zomb[0]);
    for (int i = 0; i < sum_zmb; i++) {
        if (zomb[i].used) {
            if (zomb[i].dead == false) {
                zomb[i].x -= zomb[i].speed;
                zomb[i].fps_index = (zomb[i].fps_index + 1) % 22;
                if (zomb[i].x < 170) {
                    gameState = LOSE;
                    return;
                }
                if (zomb[i].eating) {
                    zomb[i].eating_index = (zomb[i].eating_index + 1) % 22;
                }
            }
            else {
                zomb[i].dead_index++;
                if (zomb[i].dead_index >= 20) {
                    zomb[i].used = false;
                }
            }
        }
    }

}
void update_shoot() {
    int count_b = sizeof(bts) / sizeof(bts[0]);
    for (int i = 0; i < count_b; i++) {
        if (bts[i].used) {
            bts[i].x += bts[i].speed;
            if (bts[i].x > width) {
                bts[i].used = false;
            }
            if (bts[i].blast) {
                bts[i].fps_index++;
                if (bts[i].fps_index > 3) {
                    bts[i].blast = false;
                    //
                    bts[i].used = false;
                }
            }
        }
    }
}
void shoot() {
    int sign[3] = { 0 };
    int count_zmb = sizeof(zomb) / sizeof(zomb[0]);
    int X = width - zomb_img[0].getwidth() / 2;
    for (int i = 0; i < count_zmb; i++) {
        if (zomb[i].used && zomb[i].x <= X && zomb[i].x > 250) {
            sign[zomb[i].row - 1] = true;
        }
    }
    static int counts[9][3] = { 0 };
    int bts_max = sizeof(bts) / sizeof(bts[0]);
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 3; j++) {
            if (map[i][j].type == F1 && sign[j]) {
                counts[i][j]++;
                if (counts[i][j] >= 40) {
                    counts[i][j] = 0;
                    int k = 0;
                    for (; k < bts_max && bts[k].used; k++);

                    if (k < bts_max) {
                        bts[k].used = true;
                        bts[k].row = j;
                        bts[k].speed = 8;
                        int X = 256 + i * 81;
                        int Y = 179 + j * 102 + 14;
                        bts[k].x = X + moving_card[map[i][j].type][0].getwidth() - 10;
                        bts[k].y = Y;
                        bts[k].fps_index = 0;
                        bts[k].blast = false; // 确保新子弹不是爆炸状态
                    }
                }
            }
        }
    }
}
void cheak_collision_shoot() {
    int zomb_max = sizeof(zomb) / sizeof(zomb[0]);
    int bts_max = sizeof(bts) / sizeof(bts[0]);
    for (int i = 0; i < bts_max; i++) {
        if (!bts[i].used || bts[i].blast)  continue;
        for (int j = 0; j < zomb_max; j++) {
            if (!zomb[j].used)   continue;
            int x1 = zomb[j].x + 80;
            int x2 = zomb[j].x + 110;
            if (bts[i].row + 1 == zomb[j].row && bts[i].x > x1 && bts[i].x < x2) {
                zomb[j].hp -= 20;
                bts[i].blast = true;
                bts[i].speed = 0;
                if (zomb[j].hp <= 0) {
                    zomb[j].dead = true;
                    zomb[j].dead_index = 0;
                    zomb[j].speed = 0;
                }
                printf("Bullet[%d] collision at (%d,%d), Zombie[%d] HP=%d\n",
                    i, bts[i].x, bts[i].y, j, zomb[j].hp);
            }
        }
    }
}
void cheak_collision_eat() {
    int zomb_count = sizeof(zomb) / sizeof(zomb[0]);
    for (int i = 0; i < zomb_count; i++) {
        if (zomb[i].dead) continue;

        // 坐标系转换：僵尸行号转地图行索引（1-3转0-2）
        int map_row = zomb[i].row - 1;
        if (map_row < 0 || map_row >= 3) continue;

        // 从右向左遍历植物列（检测最近植物）
        for (int map_col = 8; map_col >= 0; map_col--) {
            if (map[map_col][map_row].type == -1) continue;
            // 计算植物碰撞区域（中心50像素）
            int plant_left = 256 + map_col * 81 + 30;  // X坐标+15像素
            int plant_right = plant_left + 50;         // 总宽度50像素

            // 计算僵尸攻击点（取头部位置：x+110）
            int zombie_attack_point = zomb[i].x + 110;

            // 碰撞检测：当僵尸头部进入植物防御区域
            if (zombie_attack_point > plant_left &&
                zombie_attack_point < plant_right) {

                if (map[map_col][map_row].ate) {
                    // 持续啃食
                    if (--map[map_col][map_row].hp <= 0) {
                        // 植物死亡处理
                        map[map_col][map_row].type = -1;
                        map[map_col][map_row].ate = false;
                        zomb[i].eating = false;
                        zomb[i].speed = 1; // 恢复移动
                    }
                }
                else {
                    // 开始啃食
                    map[map_col][map_row].ate = true;
                    map[map_col][map_row].hp = 100;
                    zomb[i].eating = true;
                    zomb[i].speed = 0;     // 停止移动
                    zomb[i].eating_index = 0;
                }
                break;
            }
        }
    }
}
void check_collision() {
    cheak_collision_shoot();
    cheak_collision_eat();
}
void update_dance() {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 3; j++) {
            if (map[i][j].type >= 0) {
                int type = map[i][j].type;

                // 向日葵生产阳光逻辑（F2）
                if (type == F2) {
                    map[i][j].sun_timer++;
                    if (map[i][j].sun_timer >= 200) { 
                        // 在植物位置生成阳光
                        int pool_size = sizeof(sun_pool) / sizeof(sun_pool[0]);
                        for (int k = 0; k < pool_size; k++) {
                            if (!sun_pool[k].used && !sun_pool[k].y_off) {
                                // 计算向日葵屏幕坐标
                                int plant_x = 256 + i * 81 + 30;
                                int plant_y = 179 + j * 102 + 50;

                                sun_pool[k].x = plant_x + rand() % 30 - 15;
                                sun_pool[k].y = plant_y;
                                sun_pool[k].end_y = plant_y + 80 + rand() % 40;
                                sun_pool[k].used = true;
                                sun_pool[k].timer = 0;
                                sun_pool[k].fps_index = 0;
                                mciSendString("play C:/Users/amd/Desktop/植物大战僵尸/res/sunshine.mp3", 0, 0, 0);
                                map[i][j].sun_timer = 0;
                                break;
                            }
                        }
                    }
                }
                if (total_frames[type] > 0) {
                    map[i][j].fps_index = (map[i][j].fps_index + 1) % total_frames[type];
                }
            }
        }
    }
    sunshine_ini();
    sunshine_down();
    crate_zomb();
    update_zomb();
    shoot();
    update_shoot();
    check_collision();
}
void collect_sunlight(ExMessage msg) {
    int sun_w = sun_img[0].getwidth();  int sun_h = sun_img[0].getheight();
    int count = sizeof(sun_pool) / sizeof(sun_pool[0]);
    for (int i = 0; i < count; i++) {
        if (sun_pool[i].used) {
            int x = sun_pool[i].x;
            int y = sun_pool[i].y;
            if (msg.x >= x && msg.x <= x + sun_w && msg.y >= y && msg.y <= y + sun_h) {
                mciSendString("play C:/Users/amd/Desktop/植物大战僵尸/res/sunshine.mp3", 0, 0, 0);
                sun_pool[i].used = false;
                //sun_value += 25;
                float x1 = 262, y1 = 0;
                float alpha = atan((y - y1) / (x - x1));
                sun_pool[i].x_off = 16 * cos(alpha);
                sun_pool[i].y_off = 16 * sin(alpha);
            }
        }
    }
}
void user_cli() {
    ExMessage msg;
    cli_up = false;
    cli_down = false;
    if (peekmessage(&msg)) {
        if (msg.message == WM_LBUTTONDOWN) {
            cli_down = true;
            if (msg.x >= 323 && msg.x <= (323 + 65 * count) && msg.y <= 92) {
                int index = (msg.x - 323) / 65;
                std::cout << index << std::endl;
                sign_index = index;
            }
            else {
                collect_sunlight(msg);
            }
        }
        else if (msg.message == WM_MOUSEMOVE) {
            //点击成功后，标记移动时的位置，然后update
            if (sign_index != -1) {
                moving_x = msg.x;   moving_y = msg.y;
                if_moving = true;
            }
        }
        else if (msg.message == WM_LBUTTONUP) {
            //种下植物
            cli_up = true;
            int i_index, j_index;
            i_index = (msg.x - 250) / 83;
            j_index = (msg.y - 175) / 103;
            std::cout << i_index << " " << j_index << std::endl;
            if (map[i_index][j_index].type == -1&&sign_index!=-1) {
                int cost = SUN_COST[sign_index];
                if (sun_value >= cost) {
                    sun_value -= cost;
                    map[i_index][j_index].type = sign_index;
                    map[i_index][j_index].fps_index = 0;
                }
            }
            sign_index = -1;
        }
    }
}
void startUI() {
    IMAGE start_ui, menu1, menu2;
    loadimage(&start_ui, "C:/Users/amd/Desktop/植物大战僵尸/res/menu.png");
    loadimage(&menu1, "C:/Users/amd/Desktop/植物大战僵尸/res/menu1.png");
    loadimage(&menu2, "C:/Users/amd/Desktop/植物大战僵尸/res/menu2.png");
    bool shot = false;
    while (1) {
        BeginBatchDraw();
        putimage(0, 0, &start_ui);
        if (!shot)    new_png(NULL, 475, 75, &menu1);
        else    new_png(NULL, 475, 75, &menu2);
        ExMessage mesg;
        //331 150
        if (peekmessage(&mesg)) {
            if (mesg.message == WM_LBUTTONDOWN) {
                if (mesg.x >= 475 && mesg.x <= 475 + 331 && mesg.y >= 75 && mesg.y <= 75 + 150) {
                    shot = true;
                }
            }
            else if (mesg.message == WM_LBUTTONUP && shot) {
                return;
            }
        }
        EndBatchDraw();
    }
}
void bg_music() {
    mciSendString("open C:/Users/amd/Desktop/植物大战僵尸/res/bg.mp3 alias BGM", 0, 0, 0);
    mciSendString("play BGM repeat", 0, 0, 0); // 循环播放
    // 在音效播放处添加关闭指令
    mciSendString("stop SunshineFX", 0, 0, 0);
    mciSendString("close SunshineFX", 0, 0, 0);
    mciSendString("play C:/Users/amd/Desktop/植物大战僵尸/res/sunshine.mp3 alias SunshineFX", 0, 0, 0);
}
int main() {
    init();
    startUI();
    bg_music();
    using clock = std::chrono::steady_clock;
    auto last_frame_time = clock::now();
    auto last_animate_time = clock::now();
    constexpr auto frame_duration = std::chrono::milliseconds(1000 / fps); // 40ms for 25fps
    constexpr auto animate_interval = std::chrono::milliseconds(50); // 控制动画速度
    while (1) {
        user_cli();
        // 获取当前时间
        auto now = clock::now();
        // 更新渲染（固定帧率）
        if (now - last_frame_time >= frame_duration) {
            update();
            last_frame_time = now;
        }
        // 更新动画（固定动画速度）
        if (now - last_animate_time >= animate_interval) {
            update_dance();
            last_animate_time = now;
        }
    }
    system("pause");
    return 0;
}