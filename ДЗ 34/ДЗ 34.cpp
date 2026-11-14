#include <iostream>
#include <string>
#include <algorithm>
#include <windows.h>
using namespace std;

class Damageable {
public:
    virtual void takeDamage(int damage) = 0;
    virtual bool isDestroyed() const = 0;
    virtual ~Damageable() = default;
};

class Hero {
    string name;
    int hp, maxHp, power, gold = 0, keys = 0;
public:
    Hero(const string& n, int h, int m, int p)
        : name(n), hp(h), maxHp(m), power(p) {
    }

    string getName() const { return name; }
    void addGold(int n) { gold += n; }
    void addKey() { keys++; }

    bool spendGold(int n) {
        if (gold < n) return false;
        gold -= n;
        return true;
    }

    bool useKey() {
        if (!keys) return false;
        keys--;
        return true;
    }

    void heal(int n) {
        hp = min(hp + n, maxHp);
    }

    void attack(Damageable& target) {
        cout << name << " бьёт: " << power << " урона\n";
        target.takeDamage(power);
    }

    void print() const {
        cout << name << ": hp " << hp << "/" << maxHp
            << ", золото " << gold << ", ключей " << keys << '\n';
    }
};

class Interactable {
public:
    virtual string describe() const = 0;
    virtual void interact(Hero& hero) = 0;
    virtual ~Interactable() = default;
};

class Door : public Interactable, public Damageable {
    string name;
    bool locked;
    int durability;
public:
    Door(const string& n, bool l, int d)
        : name(n), locked(l), durability(d) {
    }

    string describe() const override {
        return name + (locked ? " (закрыты)" : " (открыты)");
    }

    void interact(Hero& h) override {
        if (!locked)
            cout << "Вы проходите дальше\n";
        else if (h.useKey()) {
            locked = false;
            cout << "Ключ подошёл! " << name << " открыты\n";
        }
        else cout << "Закрыто. Нужен ключ\n";
    }

    void takeDamage(int d) override {
        durability = max(0, durability - d);
        if (isDestroyed()) {
            locked = false;
            cout << name << " выломаны!\n";
        }
    }

    bool isDestroyed() const override {
        return durability == 0;
    }
};

class Chest : public Interactable {
    int gold;
    bool hasKeyInside, opened = false;
public:
    Chest(int g, bool k) : gold(g), hasKeyInside(k) {}

    string describe() const override {
        return opened ? "Сундук (пустой)" : "Сундук";
    }

    void interact(Hero& h) override {
        if (opened) {
            cout << "Сундук пуст\n";
            return;
        }
        opened = true;
        h.addGold(gold);
        cout << "Найдено золото: " << gold << '\n';
        if (hasKeyInside) {
            h.addKey();
            cout << "Найден ключ\n";
        }
    }
};

class Fountain : public Interactable {
    int healAmount;
public:
    Fountain(int n) : healAmount(n) {}

    string describe() const override {
        return "Фонтан исцеления";
    }

    void interact(Hero& h) override {
        h.heal(healAmount);
        cout << "Вы пьёте из фонтана: +" << healAmount << " hp\n";
    }
};

class Npc : public Interactable {
protected:
    string name, phrase;
public:
    Npc(const string& n, const string& p) : name(n), phrase(p) {}

    string describe() const override { return name; }

    void greet() const {
        cout << name << ": \"" << phrase << "\"\n";
    }
};

class Merchant : public Npc {
    int potionPrice, potionHeal;
public:
    Merchant(const string& n, int p, int h)
        : Npc(n, "Купите зелье!"), potionPrice(p), potionHeal(h) {
    }

    void interact(Hero& h) override {
        greet();
        if (h.spendGold(potionPrice)) {
            h.heal(potionHeal);
            cout << "Вы купили зелье: +" << potionHeal << " hp\n";
        }
        else cout << "Не хватает золота\n";
    }
};

class Healer : public Npc {
    bool helped = false;
public:
    Healer(const string& n) : Npc(n, "Я помогу тебе") {}

    void interact(Hero& h) override {
        greet();
        if (!helped) {
            h.heal(50);
            helped = true;
            cout << "Целительница лечит вас: +50 hp\n";
        }
        else cout << "Я уже помогла тебе сегодня\n";
    }
};

class Barrel : public Damageable {
    int durability;
public:
    Barrel(int d) : durability(d) {}

    void takeDamage(int d) override {
        durability = max(0, durability - d);
        if (isDestroyed()) cout << "Бочка разбита\n";
    }

    bool isDestroyed() const override {
        return durability == 0;
    }
};

void use(Hero& h, Interactable& obj) {
    cout << "\n> " << obj.describe() << '\n';
    obj.interact(h);
    h.print();
}

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    Hero conan("Конан", 60, 100, 10);
    Door door("Дубовые двери", true, 50);
    Chest chest(30, true);
    Fountain fountain(25);

    use(conan, door);
    use(conan, chest);
    use(conan, door);
    use(conan, chest);
    use(conan, fountain);

    Hero lyra("Лира", 30, 100, 10);

    Interactable** room = new Interactable * [5] {
        new Chest(20, false),
            new Merchant("Гном Борин", 15, 40),
            new Healer("Сестра Мирна"),
            new Merchant("Гном Борин", 15, 40),
            new Fountain(10)
        };

    for (int i = 0; i < 5; i++) use(lyra, *room[i]);
    for (int i = 0; i < 5; i++) delete room[i];
    delete[] room;

    Door ironDoor("Железные двери", true, 30);
    Barrel barrel(15);

    use(conan, ironDoor);

    while (!ironDoor.isDestroyed())
        conan.attack(ironDoor);

    use(conan, ironDoor);

    while (!barrel.isDestroyed())
        conan.attack(barrel);

    Interactable* asObject = &ironDoor;
    Damageable* asTarget = &ironDoor;

    cout << asObject->describe() << '\n';
    cout << "Разрушены: " << boolalpha << asTarget->isDestroyed();

    return 0;
}

/*
1. Все эти классы реализуют Interactable.
   Благодаря полиморфизму вызывается нужный метод.

2. У объектов разные поля и назначение, но есть
   общее действие. Интерфейс объединяет их по возможностям.

3. Интерфейс задаёт требования к методам.
   Npc дополнительно хранит общие данные и готовые методы.

4. В Npc остался чисто виртуальный interact().
   Если его не переопределить в Healer,
   этот класс тоже останется абстрактным.

5. Ни одной строки. Новые классы тоже реализуют
   интерфейс Interactable.

6. Чтобы правильно удалять наследников через указатель
   на интерфейс. В программе это delete room[i].

7. Через asObject доступны describe() и interact().
   Через asTarget — takeDamage() и isDestroyed().
   Ромба здесь нет, потому что интерфейсы
   не имеют общего базового класса.
*/
