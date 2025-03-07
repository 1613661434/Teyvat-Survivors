#include <graphics.h>
#include <iostream>
#include <string>
#include <windows.h>
#include <conio.h>
#include <ctime>
#include <random>
#include <vector>
#include "putimage_alpha.h"
//#include "tools.h"
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define BUTTON_WIDTH 192
#define BUTTON_HEIGHT 75

bool is_game_started = false;
bool running = true;

//废案
//#define PLAYER_ANIM_NUM 6
//int idx_current_anim = 0;
//IMAGE img_player_left[PLAYER_ANIM_NUM];
//IMAGE img_player_right[PLAYER_ANIM_NUM];
////角色图片加载
//void LoadAnimation()
//{
//	std::wstring path;
//	for (size_t i = 0; i < PLAYER_ANIM_NUM; ++i)
//	{
//		path = L"img/player_left_" + std::to_wstring(i) + L".png";
//		loadimage(&img_player_left[i], path.c_str());
//	}
//
//	for (size_t i = 0; i < PLAYER_ANIM_NUM; ++i)
//	{
//		path = L"img/player_right_" + std::to_wstring(i) + L".png";
//		loadimage(&img_player_right[i], path.c_str());
//	}
//}

//定时器，去控制自动移动的东西，先包含头文件#include <time.h>
bool Timer(int duration, int id) //参数：持续时间，几号定时器
{
	static int startTime[10];
	int endTime = clock();
	if (endTime - startTime[id] > duration)
	{
		startTime[id] = endTime;
		return true;
	}
	return false;
}

//设计模式：享元模式
class GameObject
{
public:
	IMAGE img_shadow;
	std::vector<IMAGE*> frame_list_left;
	std::vector<IMAGE*> frame_list_right;
public:
	GameObject(LPCTSTR path_left, LPCTSTR path_right, LPCTSTR path_shadow, int num)
	{
		loadimage(&img_shadow, path_shadow);
		TCHAR path_file[256];
		for (size_t i = 0; i < num; ++i)
		{
			IMAGE* frame = new IMAGE();
			_stprintf_s(path_file, path_left, i);
			loadimage(frame, path_file);
			frame_list_left.push_back(frame);

			frame = new IMAGE();
			_stprintf_s(path_file, path_right, i);
			loadimage(frame, path_file);
			frame_list_right.push_back(frame);
		}
	}

	~GameObject()
	{
		for (size_t i = 0; i < frame_list_left.size(); ++i)
		{
			delete frame_list_left[i];
			delete frame_list_right[i];
		}
	}
};

class Player
{
private:
#define PLAYER_SPEED 10
#define PLAYER_WIDTH 80
#define PLAYER_HEIGHT 80
#define P_SHADOW_WIDTH 32
	int idx_frame = 0; //动画帧索引
	int interval_ms = 0;
	POINT player_pos = { 500,500 };
	unsigned int score = 0; //玩家得分
public:	static GameObject* anim_GO;
public:
	Player(GameObject* anim_GO,int interval) :interval_ms(interval)
	{
		this->anim_GO = anim_GO;
	}

	~Player() = default;

	//角色移动
	void Move()
	{
		if (Timer(interval_ms, 0))
		{
			int dir_x = 0;
			int dir_y = 0;
			if (GetAsyncKeyState('W') & 0x8000 && !(GetAsyncKeyState('S') & 0x8000)) // 检测 W 键
			{
				dir_y = -1;
			}
			if (GetAsyncKeyState('S') & 0x8000 && !(GetAsyncKeyState('W') & 0x8000)) // 检测 S 键
			{
				dir_y = 1;
			}
			if (GetAsyncKeyState('A') & 0x8000 && !(GetAsyncKeyState('D') & 0x8000)) // 检测 A 键
			{
				dir_x = -1;
			}
			if (GetAsyncKeyState('D') & 0x8000 && !(GetAsyncKeyState('A') & 0x8000)) // 检测 D 键
			{
				dir_x = 1;
			}

			double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
			if (len_dir != 0)
			{
				double normalized_x = dir_x / len_dir;
				double normalized_y = dir_y / len_dir;
				player_pos.x += static_cast<int>(PLAYER_SPEED * normalized_x);
				player_pos.y += static_cast<int>(PLAYER_SPEED * normalized_y);
			}

			//边界
			if (player_pos.x < 0) { player_pos.x = 0; }
			if (player_pos.y < 0) { player_pos.y = 0; }
			if (player_pos.x + PLAYER_WIDTH > WINDOW_WIDTH) { player_pos.x = WINDOW_WIDTH - PLAYER_WIDTH; }
			if (player_pos.y + PLAYER_HEIGHT > WINDOW_HEIGHT) { player_pos.y = WINDOW_HEIGHT - PLAYER_HEIGHT; }
		}
	}

	//游戏进行时的绘制
	void Draw()
	{
		int pos_shadow_x = player_pos.x + ((PLAYER_WIDTH - P_SHADOW_WIDTH) >> 1);
		int pos_shadow_y = player_pos.y + PLAYER_HEIGHT - 8;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &anim_GO->img_shadow);
		static bool facing_left = false;
		if (GetAsyncKeyState('A') & 0x8000 && !(GetAsyncKeyState('D') & 0x8000)) { facing_left = true; }
		else if (GetAsyncKeyState('D') & 0x8000 && !(GetAsyncKeyState('A') & 0x8000)) { facing_left = false; }

		if (Timer(interval_ms, 1)) { idx_frame = (idx_frame + 1) % anim_GO->frame_list_left.size(); }

		if (facing_left) { putimage_alpha(player_pos.x, player_pos.y, anim_GO->frame_list_left[idx_frame]); }
		else { putimage_alpha(player_pos.x, player_pos.y, anim_GO->frame_list_right[idx_frame]); }
	}

	//绘制玩家得分
	void DrawPlayerScore()
	{
		static TCHAR text[64];
		_stprintf_s(text, _T("当前玩家得分:%d"), score);

		setbkmode(TRANSPARENT);
		settextcolor(RGB(255, 85, 185));
		outtextxy(10, 10, text);
	}

	const POINT& GetPosition() const
	{
		return player_pos;
	}

	const int GetPlayerWidth() const
	{
		return PLAYER_WIDTH;
	}

	const int GetPlayerHeight() const
	{
		return PLAYER_HEIGHT;
	}

	void AddScore(unsigned int score)
	{
		this->score += score;
	}

	unsigned int GetScore() const
	{
		return score;
	}
};
GameObject* Player::anim_GO = nullptr; // 类外定义

class Bullet
{
public:
#define RADIUS 10
	POINT position = { 0,0 };
public:
	Bullet() = default;

	~Bullet() = default;
	
	void Draw() const
	{
		setlinecolor(RED);
		setfillcolor(RGB(255, 155, 50));
		fillcircle(position.x, position.y, RADIUS);
	}
};

class Enemy
{
private:
#define ENEMY_SPEED 2
#define ENEMY_WIDTH 80
#define ENEMY_HEIGHT 80
#define E_SHADOW_WIDTH 48
	int idx_frame = 0; //动画帧索引
	int interval_ms = 0;
	POINT enemy_pos = { 0,0 };
	bool facing_left = false;
	bool alive = true;
	int start_time;  // 每个敌人独立计时器
public:	static GameObject* enemy_GO;
public:
	Enemy(GameObject* enemy_GO, int interval) :interval_ms(interval), start_time(clock())
	{
		this->enemy_GO = enemy_GO;

		//敌人生成边界
		enum class SpawnEdge: char
		{
			Up = 0, Dwon = 1, Left = 2, Right = 3
		};

		static std::random_device rd; //随机数引擎
		static std::uniform_int_distribution<int> dist(0, 3); //均匀分布0到3之间的整数
		static std::uniform_real_distribution<double> dist2(0.0, WINDOW_WIDTH); //均匀分布0到1280之间的实数
		static std::uniform_real_distribution<double> dist3(0.0, WINDOW_HEIGHT); //均匀分布0到720之间的实数

		//将敌人位置放置在地图边界外的随机位置
		SpawnEdge edge = (SpawnEdge)dist(rd);
		switch (edge)
		{
		case SpawnEdge::Up:
			enemy_pos.x = dist2(rd);
			enemy_pos.y = 0;
			break;
		case SpawnEdge::Dwon:
			enemy_pos.x = dist2(rd);
			enemy_pos.y = WINDOW_HEIGHT;
			break;
		case SpawnEdge::Left:
			enemy_pos.x = 0;
			enemy_pos.y = dist3(rd);
			break;
		case SpawnEdge::Right:
			enemy_pos.x = WINDOW_WIDTH;
			enemy_pos.y = dist3(rd);
			break;
		default:
			break;
		}
	}

	~Enemy() = default;

	//检测敌人与子弹碰撞
	bool CheckBulletCollision(const Bullet& bullet)
	{
		//将子弹等效为点，判断点是否在敌人矩形内
		bool is_overlap_x = enemy_pos.x <= bullet.position.x && bullet.position.x <= enemy_pos.x + ENEMY_WIDTH;
		bool is_overlap_y = enemy_pos.y <= bullet.position.y && bullet.position.y <= enemy_pos.y + ENEMY_HEIGHT;
		return is_overlap_x && is_overlap_y;
	}

	//检测敌人与玩家碰撞
	bool CheckPlayerCollision(const Player& player)
	{
		//将敌人中心位置等效为点，判断点是否在玩家矩形内
		POINT check_position = { enemy_pos.x + ENEMY_WIDTH / 2,enemy_pos.y + ENEMY_HEIGHT / 2 };
		POINT player_pos = player.GetPosition();
		bool is_overlap_x = player_pos.x <= check_position.x && check_position.x <= player_pos.x + ENEMY_WIDTH;
		bool is_overlap_y = player_pos.y <= check_position.y && check_position.y <= player_pos.y + ENEMY_HEIGHT;
		return is_overlap_x && is_overlap_y;
	}

	void Move(const Player& player)
	{
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - enemy_pos.x;
		int dir_y = player_position.y - enemy_pos.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			enemy_pos.x += (int)(ENEMY_SPEED * normalized_x);
			enemy_pos.y += (int)(ENEMY_SPEED * normalized_y);
		}

		if (dir_x < 0)
		{
			facing_left = true;
		}
		else if (dir_x > 0)
		{
			facing_left = false;
		}
	}

	void Draw()
	{
		// 使用独立计时器判断动画帧
		if (clock() - start_time > interval_ms) {
			idx_frame = (idx_frame + 1) % enemy_GO->frame_list_left.size();
			start_time = clock();
		}

		// 绘制阴影
		int pos_shadow_x = enemy_pos.x + ((ENEMY_WIDTH - E_SHADOW_WIDTH) >> 1);
		int pos_shadow_y = enemy_pos.y + ENEMY_HEIGHT - 35;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &enemy_GO->img_shadow);

		// 绘制本体
		if (facing_left) { putimage_alpha(enemy_pos.x, enemy_pos.y, enemy_GO->frame_list_left[idx_frame]); }
		else { putimage_alpha(enemy_pos.x, enemy_pos.y, enemy_GO->frame_list_right[idx_frame]); }
	}

	static void TryGenerateEnemy(std::vector<Enemy*>& enemy_list)
	{
		if (rand() % 100 < 5) // 每帧3%概率生成
		{
			enemy_list.push_back(new Enemy(enemy_GO, 29));
		}
	}
};
GameObject* Enemy::enemy_GO = nullptr; // 类外定义

class Button
{
private:
	enum class Status
	{
		Idle = 0,
		Hovered = 1,
		Pushed = 2
	};
private:
	RECT region;
	IMAGE img_idle;
	IMAGE img_hovered;
	IMAGE img_pushed;
	Status status = Status::Idle;
private:
	//检测鼠标点击
	bool CheckCursorHit(int x, int y)
	{
		return region.left <= x && x <= region.right && region.top <= y && y <= region.bottom;
	}
protected:
	virtual void OnClick() = 0;
public:
	Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
	{
		region = rect;

		loadimage(&img_idle, path_img_idle);
		loadimage(&img_hovered, path_img_hovered);
		loadimage(&img_pushed, path_img_pushed);
	}

	~Button() = default;

	void Draw()
	{
		switch (status)
		{
		case Status::Idle:
			putimage(region.left, region.top, &img_idle);
			break;
		case Status::Hovered:
			putimage(region.left, region.top, &img_hovered);
			break;
		case Status::Pushed:
			putimage(region.left, region.top, &img_pushed);
			break;
		}
	}

	void ProcessEvent(const ExMessage& msg)
	{
		switch (msg.message)
		{
		case WM_MOUSEMOVE:
			if (status == Status::Idle && CheckCursorHit(msg.x, msg.y))
			{
				status = Status::Hovered;
			}
			else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
			{
				status = Status::Idle;
			}
			break;
		case WM_LBUTTONDOWN:
			if (CheckCursorHit(msg.x, msg.y))
			{
				status = Status::Pushed;
			}
			break;
		case WM_LBUTTONUP:
			if (status == Status::Pushed)
			{
				OnClick();
			}
			break;
		default:
			break;
		}
	}
};

//开始游戏按钮
class StartGameButton :public Button
{
protected:
	void OnClick()
	{
		is_game_started = true;
		//播放背景音乐
		mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);
	}
public:
	StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed){ }
	~StartGameButton() = default;
};

//退出游戏按钮
class QuitGameButton :public Button
{
protected:
	void OnClick()
	{
		running = false;
	}
public:
	QuitGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {
	}
	~QuitGameButton() = default;
};

//更新子弹位置
void UpdateBullets(std::vector<Bullet>& bullet_list, const Player& player)
{
#define RADIAL_SPEED 0.0045 //径向波动速度
#define TANGENT_SPEED 0.0055 //切向波动速度
	double radian_interval = 2 * 3.14159 / bullet_list.size(); //子弹之间的弧度间隔
	POINT player_position = player.GetPosition();
	double radius = 100 + 25 * sin(clock() * RADIAL_SPEED);
	for (size_t i = 0; i < bullet_list.size(); ++i)
	{
		double radian = clock() * TANGENT_SPEED + radian_interval * i; //当前子弹所在的弧度值
		bullet_list[i].position.x = player_position.x + player.GetPlayerWidth() / 2 + (int)(radius * sin(radian));
		bullet_list[i].position.y = player_position.y + player.GetPlayerHeight() / 2 + (int)(radius * cos(radian));
	}
}

int main()
{
	// 获取屏幕分辨率
	const int screen_width = GetSystemMetrics(SM_CXSCREEN);
	const int screen_height = GetSystemMetrics(SM_CYSCREEN);

	// 计算窗口起始坐标
	const int start_x = (screen_width - WINDOW_WIDTH) / 2;
	const int start_y = (screen_height - WINDOW_HEIGHT) / 2;

	// 创建窗口
	HWND hwnd = initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

	// 设置窗口位置
	SetWindowPos(hwnd, NULL, start_x, start_y,
		WINDOW_WIDTH, WINDOW_HEIGHT,
		SWP_NOZORDER | SWP_NOACTIVATE);

	//初始化资产指针
	Player::anim_GO = new GameObject(_T("img/player_left_%d.png"), _T("img/player_right_%d.png"), _T("img/shadow_player.png"), 6);
	Enemy::enemy_GO = new GameObject(_T("img/enemy_left_%d.png"), _T("img/enemy_right_%d.png"), _T("img/shadow_enemy.png"), 6);

	//创建角色对象
	Player anim(Player::anim_GO, 29);

	//加载背景音乐
	mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);
	//加载击中音效
	mciSendString(_T("open mus/hit.wav alias hit"), NULL, 0, NULL);

	//子弹类线性表
	std::vector<Bullet> bullet_list(3);

	//敌人类指针线性表
	std::vector<Enemy*> enemy_list;
	BeginBatchDraw();

	//按钮相关
	RECT region_btn_start_game, region_btn_quit_game;

	region_btn_start_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_btn_start_game.right = region_btn_start_game.left + BUTTON_WIDTH;
	region_btn_start_game.top = 430;
	region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;

	region_btn_quit_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_btn_quit_game.right = region_btn_quit_game.left + BUTTON_WIDTH;
	region_btn_quit_game.top = 550;
	region_btn_quit_game.bottom = region_btn_quit_game.top + BUTTON_HEIGHT;

	//开始游戏及退出游戏
	StartGameButton btn_start_game = StartGameButton(region_btn_start_game,
		_T("img/ui_start_idle.png"), _T("img/ui_start_hovered.png"), _T("img/ui_start_pushed.png"));
	QuitGameButton btn_quit_game = QuitGameButton(region_btn_quit_game,
		_T("img/ui_quit_idle.png"), _T("img/ui_quit_hovered.png"), _T("img/ui_quit_pushed.png"));

	//加载主菜单背景图
	IMAGE img_menu;
	loadimage(&img_menu, _T("img/menu.png"));

	//加载开始游戏背景
	IMAGE img_background;
	loadimage(&img_background, _T("img/background.png"));

	while (running)
	{
		int start_time = clock();

		ExMessage msg;

		while (!is_game_started && peekmessage(&msg))
		{
			btn_start_game.ProcessEvent(msg);
			btn_quit_game.ProcessEvent(msg);
		}

		if (is_game_started)
		{
			//角色移动
			anim.Move();

			//更新子弹位置
			UpdateBullets(bullet_list, anim);

			//敌人生成
			Enemy::TryGenerateEnemy(enemy_list);
			for (Enemy* enemy : enemy_list)
			{
				enemy->Move(anim);
			}

			//检测敌人与玩家的碰撞
			auto it = enemy_list.begin();
			while (it != enemy_list.end())
			{
				bool hit = false;

				// 检测子弹碰撞
				for (const Bullet& bullet : bullet_list)
				{
					if ((*it)->CheckBulletCollision(bullet))
					{
						hit = true;
						//播放击中音效
						mciSendString(_T("play hit from 0"), NULL, 0, NULL);
						anim.AddScore(1); //得分+1
						break;
					}
				}

				// 处理碰撞结果
				if (hit)
				{
					delete* it;       // 释放内存
					it = enemy_list.erase(it);  // 安全删除元素
				}
				else
				{
					// 玩家碰撞检测
					if ((*it)->CheckPlayerCollision(anim))
					{
						static TCHAR text[128];
						_stprintf_s(text, _T("最终得分:%d !"), anim.GetScore());
						MessageBox(GetHWnd(), text, _T("游戏结束"), MB_OK);
						running = false;
						break;
					}
					++it;
				}
			}
		}

		cleardevice();

		if (is_game_started)
		{
			//绘画背景
			putimage(0, 0, &img_background);
			//绘画角色
			anim.Draw();
			//绘画敌人
			for (Enemy* enemy : enemy_list)
			{
				enemy->Draw();
			}
			//绘画子弹
			for (const Bullet& bullet : bullet_list)
			{
				bullet.Draw();
			}
			//绘制玩家得分
			anim.DrawPlayerScore();
		}
		else
		{
			putimage(0, 0, &img_menu);
			btn_start_game.Draw();
			btn_quit_game.Draw();
		}

		FlushBatchDraw();

		int end_time = clock();
		int delete_time = end_time - start_time;
		if (delete_time < 17) // 约60FPS
		{
			Sleep(17 - delete_time);
		}
	}

	delete Player::anim_GO;
	delete Enemy::enemy_GO;

	EndBatchDraw();
	return 0;
}