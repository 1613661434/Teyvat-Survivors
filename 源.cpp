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

//�ϰ�
//#define PLAYER_ANIM_NUM 6
//int idx_current_anim = 0;
//IMAGE img_player_left[PLAYER_ANIM_NUM];
//IMAGE img_player_right[PLAYER_ANIM_NUM];
////��ɫͼƬ����
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

//��ʱ����ȥ�����Զ��ƶ��Ķ������Ȱ���ͷ�ļ�#include <time.h>
bool Timer(int duration, int id) //����������ʱ�䣬���Ŷ�ʱ��
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

//���ģʽ����Ԫģʽ
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
	int idx_frame = 0; //����֡����
	int interval_ms = 0;
	POINT player_pos = { 500,500 };
	unsigned int score = 0; //��ҵ÷�
public:	static GameObject* anim_GO;
public:
	Player(GameObject* anim_GO,int interval) :interval_ms(interval)
	{
		this->anim_GO = anim_GO;
	}

	~Player() = default;

	//��ɫ�ƶ�
	void Move()
	{
		if (Timer(interval_ms, 0))
		{
			int dir_x = 0;
			int dir_y = 0;
			if (GetAsyncKeyState('W') & 0x8000 && !(GetAsyncKeyState('S') & 0x8000)) // ��� W ��
			{
				dir_y = -1;
			}
			if (GetAsyncKeyState('S') & 0x8000 && !(GetAsyncKeyState('W') & 0x8000)) // ��� S ��
			{
				dir_y = 1;
			}
			if (GetAsyncKeyState('A') & 0x8000 && !(GetAsyncKeyState('D') & 0x8000)) // ��� A ��
			{
				dir_x = -1;
			}
			if (GetAsyncKeyState('D') & 0x8000 && !(GetAsyncKeyState('A') & 0x8000)) // ��� D ��
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

			//�߽�
			if (player_pos.x < 0) { player_pos.x = 0; }
			if (player_pos.y < 0) { player_pos.y = 0; }
			if (player_pos.x + PLAYER_WIDTH > WINDOW_WIDTH) { player_pos.x = WINDOW_WIDTH - PLAYER_WIDTH; }
			if (player_pos.y + PLAYER_HEIGHT > WINDOW_HEIGHT) { player_pos.y = WINDOW_HEIGHT - PLAYER_HEIGHT; }
		}
	}

	//��Ϸ����ʱ�Ļ���
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

	//������ҵ÷�
	void DrawPlayerScore()
	{
		static TCHAR text[64];
		_stprintf_s(text, _T("��ǰ��ҵ÷�:%d"), score);

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
GameObject* Player::anim_GO = nullptr; // ���ⶨ��

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
	int idx_frame = 0; //����֡����
	int interval_ms = 0;
	POINT enemy_pos = { 0,0 };
	bool facing_left = false;
	bool alive = true;
	int start_time;  // ÿ�����˶�����ʱ��
public:	static GameObject* enemy_GO;
public:
	Enemy(GameObject* enemy_GO, int interval) :interval_ms(interval), start_time(clock())
	{
		this->enemy_GO = enemy_GO;

		//�������ɱ߽�
		enum class SpawnEdge: char
		{
			Up = 0, Dwon = 1, Left = 2, Right = 3
		};

		static std::random_device rd; //���������
		static std::uniform_int_distribution<int> dist(0, 3); //���ȷֲ�0��3֮�������
		static std::uniform_real_distribution<double> dist2(0.0, WINDOW_WIDTH); //���ȷֲ�0��1280֮���ʵ��
		static std::uniform_real_distribution<double> dist3(0.0, WINDOW_HEIGHT); //���ȷֲ�0��720֮���ʵ��

		//������λ�÷����ڵ�ͼ�߽�������λ��
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

	//���������ӵ���ײ
	bool CheckBulletCollision(const Bullet& bullet)
	{
		//���ӵ���ЧΪ�㣬�жϵ��Ƿ��ڵ��˾�����
		bool is_overlap_x = enemy_pos.x <= bullet.position.x && bullet.position.x <= enemy_pos.x + ENEMY_WIDTH;
		bool is_overlap_y = enemy_pos.y <= bullet.position.y && bullet.position.y <= enemy_pos.y + ENEMY_HEIGHT;
		return is_overlap_x && is_overlap_y;
	}

	//�������������ײ
	bool CheckPlayerCollision(const Player& player)
	{
		//����������λ�õ�ЧΪ�㣬�жϵ��Ƿ�����Ҿ�����
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
		// ʹ�ö�����ʱ���ж϶���֡
		if (clock() - start_time > interval_ms) {
			idx_frame = (idx_frame + 1) % enemy_GO->frame_list_left.size();
			start_time = clock();
		}

		// ������Ӱ
		int pos_shadow_x = enemy_pos.x + ((ENEMY_WIDTH - E_SHADOW_WIDTH) >> 1);
		int pos_shadow_y = enemy_pos.y + ENEMY_HEIGHT - 35;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &enemy_GO->img_shadow);

		// ���Ʊ���
		if (facing_left) { putimage_alpha(enemy_pos.x, enemy_pos.y, enemy_GO->frame_list_left[idx_frame]); }
		else { putimage_alpha(enemy_pos.x, enemy_pos.y, enemy_GO->frame_list_right[idx_frame]); }
	}

	static void TryGenerateEnemy(std::vector<Enemy*>& enemy_list)
	{
		if (rand() % 100 < 5) // ÿ֡3%��������
		{
			enemy_list.push_back(new Enemy(enemy_GO, 29));
		}
	}
};
GameObject* Enemy::enemy_GO = nullptr; // ���ⶨ��

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
	//��������
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

//��ʼ��Ϸ��ť
class StartGameButton :public Button
{
protected:
	void OnClick()
	{
		is_game_started = true;
		//���ű�������
		mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);
	}
public:
	StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed){ }
	~StartGameButton() = default;
};

//�˳���Ϸ��ť
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

//�����ӵ�λ��
void UpdateBullets(std::vector<Bullet>& bullet_list, const Player& player)
{
#define RADIAL_SPEED 0.0045 //���򲨶��ٶ�
#define TANGENT_SPEED 0.0055 //���򲨶��ٶ�
	double radian_interval = 2 * 3.14159 / bullet_list.size(); //�ӵ�֮��Ļ��ȼ��
	POINT player_position = player.GetPosition();
	double radius = 100 + 25 * sin(clock() * RADIAL_SPEED);
	for (size_t i = 0; i < bullet_list.size(); ++i)
	{
		double radian = clock() * TANGENT_SPEED + radian_interval * i; //��ǰ�ӵ����ڵĻ���ֵ
		bullet_list[i].position.x = player_position.x + player.GetPlayerWidth() / 2 + (int)(radius * sin(radian));
		bullet_list[i].position.y = player_position.y + player.GetPlayerHeight() / 2 + (int)(radius * cos(radian));
	}
}

int main()
{
	// ��ȡ��Ļ�ֱ���
	const int screen_width = GetSystemMetrics(SM_CXSCREEN);
	const int screen_height = GetSystemMetrics(SM_CYSCREEN);

	// ���㴰����ʼ����
	const int start_x = (screen_width - WINDOW_WIDTH) / 2;
	const int start_y = (screen_height - WINDOW_HEIGHT) / 2;

	// ��������
	HWND hwnd = initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

	// ���ô���λ��
	SetWindowPos(hwnd, NULL, start_x, start_y,
		WINDOW_WIDTH, WINDOW_HEIGHT,
		SWP_NOZORDER | SWP_NOACTIVATE);

	//��ʼ���ʲ�ָ��
	Player::anim_GO = new GameObject(_T("img/player_left_%d.png"), _T("img/player_right_%d.png"), _T("img/shadow_player.png"), 6);
	Enemy::enemy_GO = new GameObject(_T("img/enemy_left_%d.png"), _T("img/enemy_right_%d.png"), _T("img/shadow_enemy.png"), 6);

	//������ɫ����
	Player anim(Player::anim_GO, 29);

	//���ر�������
	mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);
	//���ػ�����Ч
	mciSendString(_T("open mus/hit.wav alias hit"), NULL, 0, NULL);

	//�ӵ������Ա�
	std::vector<Bullet> bullet_list(3);

	//������ָ�����Ա�
	std::vector<Enemy*> enemy_list;
	BeginBatchDraw();

	//��ť���
	RECT region_btn_start_game, region_btn_quit_game;

	region_btn_start_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_btn_start_game.right = region_btn_start_game.left + BUTTON_WIDTH;
	region_btn_start_game.top = 430;
	region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;

	region_btn_quit_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_btn_quit_game.right = region_btn_quit_game.left + BUTTON_WIDTH;
	region_btn_quit_game.top = 550;
	region_btn_quit_game.bottom = region_btn_quit_game.top + BUTTON_HEIGHT;

	//��ʼ��Ϸ���˳���Ϸ
	StartGameButton btn_start_game = StartGameButton(region_btn_start_game,
		_T("img/ui_start_idle.png"), _T("img/ui_start_hovered.png"), _T("img/ui_start_pushed.png"));
	QuitGameButton btn_quit_game = QuitGameButton(region_btn_quit_game,
		_T("img/ui_quit_idle.png"), _T("img/ui_quit_hovered.png"), _T("img/ui_quit_pushed.png"));

	//�������˵�����ͼ
	IMAGE img_menu;
	loadimage(&img_menu, _T("img/menu.png"));

	//���ؿ�ʼ��Ϸ����
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
			//��ɫ�ƶ�
			anim.Move();

			//�����ӵ�λ��
			UpdateBullets(bullet_list, anim);

			//��������
			Enemy::TryGenerateEnemy(enemy_list);
			for (Enemy* enemy : enemy_list)
			{
				enemy->Move(anim);
			}

			//����������ҵ���ײ
			auto it = enemy_list.begin();
			while (it != enemy_list.end())
			{
				bool hit = false;

				// ����ӵ���ײ
				for (const Bullet& bullet : bullet_list)
				{
					if ((*it)->CheckBulletCollision(bullet))
					{
						hit = true;
						//���Ż�����Ч
						mciSendString(_T("play hit from 0"), NULL, 0, NULL);
						anim.AddScore(1); //�÷�+1
						break;
					}
				}

				// ������ײ���
				if (hit)
				{
					delete* it;       // �ͷ��ڴ�
					it = enemy_list.erase(it);  // ��ȫɾ��Ԫ��
				}
				else
				{
					// �����ײ���
					if ((*it)->CheckPlayerCollision(anim))
					{
						static TCHAR text[128];
						_stprintf_s(text, _T("���յ÷�:%d !"), anim.GetScore());
						MessageBox(GetHWnd(), text, _T("��Ϸ����"), MB_OK);
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
			//�滭����
			putimage(0, 0, &img_background);
			//�滭��ɫ
			anim.Draw();
			//�滭����
			for (Enemy* enemy : enemy_list)
			{
				enemy->Draw();
			}
			//�滭�ӵ�
			for (const Bullet& bullet : bullet_list)
			{
				bullet.Draw();
			}
			//������ҵ÷�
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
		if (delete_time < 17) // Լ60FPS
		{
			Sleep(17 - delete_time);
		}
	}

	delete Player::anim_GO;
	delete Enemy::enemy_GO;

	EndBatchDraw();
	return 0;
}