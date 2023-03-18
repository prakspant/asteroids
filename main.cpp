#include <SFML/Graphics.hpp>
#include <time.h>
#include <list>

using namespace sf;

const int W{ 1200 }, H{ 800 };
const float degToRad{ 0.017453f };

class Animation {
public:
	float Frame, speed;
	Sprite sprite;
	std::vector<IntRect> frames;

	Animation() {};

	Animation(Texture& t, int x, int y, int w, int h, int count, float speed) {
		Frame = 0;
		this->speed = speed;

		for (int i = 0; i < count; i++)
			frames.push_back(IntRect(x + i * w, y, w, h));

		sprite.setTexture(t);
		sprite.setOrigin(w / 2, h / 2);
		sprite.setTextureRect(frames[0]);
	}

	void update() {
		Frame += speed;
		int n = frames.size();
		if (Frame >= n) Frame -= n;
		if (n > 0) sprite.setTextureRect(frames[int(Frame)]);
	}

	bool isEnd() {
		return Frame + speed >= frames.size();
	}
};

class Entity {
public:
	float x, y, dx, dy, R, angle;
	bool life;
	std::string name;
	Animation anim;

	Entity() { life = 1; };

	void settings(Animation& a, int X, int Y, float angle = 0, int radius = 1) {
		x = X; y = Y; anim = a; 
		this->angle = angle; R = radius;
	}

	virtual void update(){};

	void draw(RenderWindow& app) {
		anim.sprite.setPosition(x, y);
		anim.sprite.setRotation(angle + 90);
		app.draw(anim.sprite);
	}
};

class asteroid : public Entity {
public:
	asteroid() {
		dx = rand() % 8 - 4;
		dy = rand() % 8 - 4;
		name = "asteroid";
	}

	void update() {
		x += dx;
		y += dy;
		if (x > W) x = 0; else if (x < 0) x = W;
		if (y > H) y = 0; else if (y < 0) y = H;
	}
};

class bullet : public Entity {
public:
	bullet()
	{
		name = "bullet";
	}
	void update() {
		dx = cos(angle * degToRad) * 6;
		dy = sin(angle * degToRad) * 6;
		x += dx;
		y += dy;
		if (x > W || x < 0 || y>H || y < 0) life = 0;
	}
};

class player : public Entity {
public:
	bool thrust;

	player() {
		name = "player";
	}

	void update() {
		if (thrust) {
			dx += cos(angle * degToRad) * 0.2;
			dy += sin(angle * degToRad) * 0.2;
		}
		else {
			dx *= 0.99;
			dy *= 0.99;
		}

		int maxSpeed = 15;
		float speed = sqrt(dx * dx + dy * dy);
		if (speed > maxSpeed) {
			dx *= maxSpeed / speed;
			dy *= maxSpeed / speed;
		}

		x += dx;
		y += dy;

		if (x > W) x = 0; else if (x < 0) x = W;
		if (y > H) y = 0; else if (y < 0) y = H;
	}
};

bool isCollide(Entity& a, Entity& b) {
	return (b.x - a.x)*(b.x - a.x) + (b.y - a.y)*(b.y - a.y) < (a.R + b.R)*(a.R + b.R);
}

int main() {
	srand(time(0));

	RenderWindow app(VideoMode(W, H), "Asteroids!");
	app.setFramerateLimit(60);

	Texture t[7];
	t[0].loadFromFile("images/spaceship.png");
	t[1].loadFromFile("images/background.jpg");
	t[2].loadFromFile("images/explosions/type_A.png");
	t[3].loadFromFile("images/rock.png");
	t[4].loadFromFile("images/fire_blue.png");
	t[5].loadFromFile("images/rock_small.png");
	t[6].loadFromFile("images/explosions/type_B.png");

	Animation sBullet(t[4], 0, 0, 32, 64, 16, 0.8);
	Animation sExplosion(t[2], 0, 0, 256, 256, 48, 0.5);
	Animation sRockSmall(t[5], 0, 0, 64, 64, 16, 0.2);
	Animation sExplosionShip(t[6], 0, 0, 192, 192, 64, 0.5);
	Animation sPlayer(t[0], 40, 0, 40, 40, 1, 0);
	Animation sPlayerGo(t[0], 40, 40, 40, 40, 1, 0);
	Animation sRock(t[3], 0, 0, 64, 64, 16, 0.2);

	Sprite sBackground(t[1]);
	sBackground.setTextureRect(IntRect(0, 0, W, H));

	std::list<Entity*> entities;

	sRock.sprite.setPosition(400, 400);

	for (int i = 0; i < 15; i++) {
		asteroid* a = new asteroid();
		a->settings(sRock, rand() % W, rand() % H, rand() % 360, 25);
		entities.push_back(a);
	}

	player* p = new player();
	p->settings(sPlayer, 200, 200, 0, 20);
	entities.push_back(p);

	while (app.isOpen()) {
		Event event;
		while (app.pollEvent(event)) {
			if (event.type == Event::Closed || Keyboard::isKeyPressed(Keyboard::Escape)) app.close();

			if (event.type == Event::KeyPressed && event.key.code == Keyboard::Space) {
				bullet* b = new bullet();
				b->settings(sBullet, p->x, p->y, p->angle, 10);
				entities.push_back(b);
			}
		}

		//sprite Animation Example

		if (Keyboard::isKeyPressed(Keyboard::D)) p->angle += 3;
		if (Keyboard::isKeyPressed(Keyboard::A)) p->angle -= 3;
		if (Keyboard::isKeyPressed(Keyboard::W)) p->thrust = true;
		else p->thrust = false;

		for(auto a:entities)
			for (auto b : entities){
				if (a->name == "asteroid" && b->name == "bullet" && isCollide(*a, *b))
				{
					a->life = false; b->life = false;
					Entity* E = new Entity();
					E->settings(sExplosion, a->x, a->y);
					E->name = "explosion";
					entities.push_back(E);

					for (int i = 0; i < 2; i++) {
						if (a->R == 15) break;
						Entity* E = new asteroid();
						E->settings(sRockSmall, a->x, a->y, rand() % 360, 15);
						entities.push_back(E);
					}
				}
				else if (a->name == "player" && b->name == "asteroid" && isCollide(*a, *b)) {
					b->life = false;

					Entity* E = new Entity();
					E->settings(sExplosionShip, a->x, a->y);
					E->name = "explosion";
					entities.push_back(E);

					p->settings(sPlayer, W / 2, H / 2, 0, 20);
					p->dx = 0; p->dy = 0;
				}
			}
		

		//spaceship movement//
		if (p->thrust) p->anim = sPlayerGo;
		else p->anim = sPlayer;

		for (auto e : entities)
			if (e->name == "explosion")
				if (e->anim.isEnd()) e->life = 0;

		if (rand() % 250 == 0) {
			asteroid* a = new asteroid();
			a->settings(sRock, 0, rand() % H, rand() % 360, 25);
			entities.push_back(a);
		}

		for (auto i = entities.begin(); i != entities.end();) {
			Entity* E = *i;
			E->update();
			E->anim.update();

			if (E->life == false) {i = entities.erase(i); delete E;}
			else i++;
		}

		//draw//
		app.clear();
		app.draw(sBackground);
		for (auto i : entities) i->draw(app);
		app.display();
	}
}