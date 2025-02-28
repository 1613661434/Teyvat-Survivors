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

class GameObject
{
public:
	IMAGE img_shadow;
	int idx_frame = 0; //动画帧索引
	std::vector<IMAGE*> frame_list_left;
	std::vector<IMAGE*> frame_list_right;
public:
	GameObject() = default;

	virtual ~GameObject()
	{
		for (size_t i = 0; i < frame_list_left.size(); ++i)
		{
			delete frame_list_left[i];
			delete frame_list_right[i];
		}
	}

	void Load(LPCTSTR path_left, LPCTSTR path_right, LPCTSTR path_shadow, int num, int interval)
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
};

class Player :public GameObject
{
private:
#define PLAYER_SPEED 10
#define PLAYER_WIDTH 80
#define PLAYER_HEIGHT 80
#define P_SHADOW_WIDTH 32
	int interval_ms = 0;
	POINT player_pos = { 500,500 };
public:
	Player(LPCTSTR path_left, LPCTSTR path_right, LPCTSTR path_shadow, int num, int interval) :interval_ms(interval)
	{
		Load(path_left, path_right, path_shadow, num, interval);
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
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);
		static bool facing_left = false;
		if (GetAsyncKeyState('A') & 0x8000 && !(GetAsyncKeyState('D') & 0x8000)) { facing_left = true; }
		else if (GetAsyncKeyState('D') & 0x8000 && !(GetAsyncKeyState('A') & 0x8000)) { facing_left = false; }

		if (Timer(interval_ms, 1)) { idx_frame = (idx_frame + 1) % frame_list_left.size(); }

		if (facing_left) { putimage_alpha(player_pos.x, player_pos.y, frame_list_left[idx_frame]); }
		else { putimage_alpha(player_pos.x, player_pos.y, frame_list_right[idx_frame]); }
	}

	POINT GetPosition() const
	{
		return player_pos;
	}
};

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
		setlinecolor(RGB(255, 155, 50));
		setfillcolor(RGB(255, 155, 50));
		fillcircle(position.x, position.y, RADIUS);
	}
};

class Enemy :public GameObject
{
private:
#define ENEMY_SPEED 2
#define ENEMY_WIDTH 80
#define ENEMY_HEIGHT 80
#define E_SHADOW_WIDTH 48
	int interval_ms = 0;
	POINT enemy_pos = { 0,0 };
	bool facing_left = false;
public:
	Enemy(LPCTSTR path_left, LPCTSTR path_right, LPCTSTR path_shadow, int num, int interval) :interval_ms(interval)
	{
		Load(path_left, path_right, path_shadow, num, interval);

		//敌人生成边界
		enum class SpawnEdge
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

	bool CheckBulletCollision(const Bullet& bullet)
	{
		return false;
	}

	bool CheckPlayerCollision(const Player& player)
	{
		return false;
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
		int pos_shadow_x = enemy_pos.x + ((ENEMY_WIDTH - E_SHADOW_WIDTH) >> 1);
		int pos_shadow_y = enemy_pos.y + ENEMY_HEIGHT - 35;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		if (Timer(interval_ms, 2)) { idx_frame = (idx_frame + 1) % frame_list_left.size(); }

		if (facing_left) { putimage_alpha(enemy_pos.x, enemy_pos.y, frame_list_left[idx_frame]); }
		else { putimage_alpha(enemy_pos.x, enemy_pos.y, frame_list_right[idx_frame]); }
	}

	static void TryGenerateEnemy(std::vector<Enemy*>& enemy_list)
	{
		if (Timer(100, 3))
		{
			enemy_list.push_back(new Enemy(_T("img/enemy_left_%d.png"), _T("img/enemy_right_%d.png"), _T("img/shadow_enemy.png"), 6, 29));
		}
	}
};

int main()
{
	initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

	bool running = true;

	IMAGE img_background;
	loadimage(&img_background, _T("img/background.png"));

	Player anim(_T("img/player_left_%d.png"), _T("img/player_right_%d.png"), _T("img/shadow_player.png"), 6, 29);
	std::vector<Enemy*>enemy_list;
	BeginBatchDraw();

	while (running)
	{
		int start_time = clock();

		anim.Move();
		Enemy::TryGenerateEnemy(enemy_list);
		for (Enemy* enemy : enemy_list)
		{
			enemy->Move(anim);
		}

		cleardevice();

		putimage(0, 0, &img_background);
		anim.Draw();
		for (Enemy* enemy : enemy_list)
		{
			enemy->Draw();
		}

		FlushBatchDraw();

		int end_time = clock();
		int delete_time = end_time - start_time;
		if (delete_time < 7)
		{
			Sleep(7 - delete_time);
		}
	}

	EndBatchDraw();
	return 0;
}