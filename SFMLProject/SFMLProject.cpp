#include "box_block.h"
#include "gjk_functions.h"
#include "collide_resolver.h"
#include "mouse_provider.h"
#include "wall_block.h"

#define screen_width 800
#define screen_height 800

vector2 center_point(0);

void draw_coordinates(sf::RenderWindow& window);

int main()
{
    std::vector<phisic_object*> phisic_objects;

    //box_block* _box = new box_block();
    //_box->size = vector2(100., 100.);
    //_box->position = vector2(-100, 0);
    ////_box->angle_velocity = - 0.6;
    ////_box->velocity.x = 2;
    ////_box->acceleration.y = 0.1;
    //
    //box_block* _box2 = new box_block();
    //_box2->size = vector2(100., 100);
    //_box2->position = vector2(100, 0);
    ////_box2->angle = 15;
    ////_box2->acceleration.y = 0.1;

    //_box2->angle = 45;
    //_box->velocity.x = 2;

    int screen_width2 = 700;
    int screen_height2 = 700;

    wall_block* left_wall = new wall_block(
        vector2(-screen_width2 / 2, screen_height2 / 2), vector2(-screen_width2 / 2, -screen_height2 / 2));
    wall_block* top_wall = new wall_block(
        vector2(-screen_width2 / 2, -screen_height2 / 2), vector2(screen_width2 / 2, -screen_height2 / 2));
    wall_block* right_wall = new wall_block(
        vector2(screen_width2 / 2 + 1, -screen_height2 / 2), vector2(screen_width2 / 2 + 1, screen_height2 / 2));
    wall_block* borrom_wall = new wall_block(
        vector2(screen_width2 / 2, screen_height2 / 2 + 1), vector2(-screen_width2 / 2, screen_height2 / 2 + 1));

    phisic_objects.emplace_back(left_wall);
    phisic_objects.emplace_back(top_wall);
    phisic_objects.emplace_back(right_wall);
    phisic_objects.emplace_back(borrom_wall);

    //phisic_objects.emplace_back(_box);
    //phisic_objects.emplace_back(_box2);

    for (int i = 0; i < 2; i++) {
        box_block* _box = new box_block();
        _box->size = vector2(200., 50.);
        _box->position = vector2(0, 55 * i);
        //_box->angle = 45;
        //_box->angle_velocity = - 0.6;
        //_box->velocity.x = 1;
        _box->acceleration.y = 0.1;
    
        phisic_objects.emplace_back(_box);
    }

    sf::RenderWindow window(sf::VideoMode(screen_width, screen_height), "SFML works!");

    sf::View view;
    sf::Vector2f center(-screen_width / 2, -screen_height / 2);
    sf::Vector2f size(screen_width, screen_height); 
    view.reset(sf::FloatRect(center, size));

    window.setView(view);
    
    primitives_drawer drawer(window);
    gjk_functions gjk(&drawer);
    collide_resolver collider_resolver(&drawer);

    mouse_provider mouse_provider(200);
    
    phisic_object* selected = NULL;
    vector2 shoulder;

    while (window.isOpen())
    {
        auto pos = sf::Mouse::getPosition(window);
        vector2 mouse_position = vector2(pos.x - (screen_width / 2), pos.y - (screen_height / 2));
        mouse_provider.taking_new_position(mouse_position);
        auto mouse_speed = mouse_provider.filtered_speed();

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                for (auto& box : phisic_objects) {
                    if (gjk.contains_point(box->points, mouse_position)) {
                        selected = box;
                        //selected->velocity = vector2(0, 0);
                        //selected->angle_velocity = 0;
                        shoulder = (mouse_position - box->position).rotate(-selected->angle);
                        break;
                    }
                }
            }
            if (event.type == sf::Event::KeyPressed) {


                if (selected != NULL) {
                    if (event.key.code == sf::Keyboard::Q) {
                        selected->angle_velocity -= 1.25;
                    }
                    if (event.key.code == sf::Keyboard::E) {
                        selected->angle_velocity += 1.25;
                    }
                    if (event.key.code == sf::Keyboard::R) {
                        selected->position = vector2();
                        selected->angle = 0;
                        selected->velocity = vector2();
                        selected->angle_velocity = 0;
                    }
                    if (event.key.code == sf::Keyboard::Space) {
                        collider_resolver.apply_impulse(*selected, selected->points[0], vector2(0, -5));
                    }
                }
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                if (selected != NULL) {
                    //selected->velocity = mouse_provider.filtered_speed();
                }
                selected = NULL;
            }
        }

        window.clear();

        for (auto& object : phisic_objects) {
            object->update_form();
        }

        for (auto& object : phisic_objects) {
             object->draw(window);
        }

        collider_resolver.resolve_collision_vector(phisic_objects);

        //drawer.draw_number(vector2(-200), collider_resolver.collide_count);
        //
        //double kinetic_energy = (_box->velocity * _box->mass).length() +
        //    _box->angle_velocity * _box->moment_of_inetia;
        //
        //drawer.draw_number(vector2(-200, -180), kinetic_energy);

        if (selected != NULL) {
            vector2 new_shoulder = shoulder.rotate(selected->angle) + selected->position;
            vector2 impulse_vector = mouse_position - new_shoulder;
            collider_resolver.apply_impulse_by_mouse(*selected, new_shoulder, impulse_vector);
        }

        draw_coordinates(window);

        window.display();
        sf::sleep(sf::seconds(0.01f));
    }

    return 0;
}

void draw_coordinates(sf::RenderWindow& window) {
    sf::Vertex line[4] =
    {
        sf::Vertex(vector2(-100, 0), sf::Color::White),
        sf::Vertex(vector2(100, 0), sf::Color::White),
        sf::Vertex(vector2(0, 100), sf::Color::White),
        sf::Vertex(vector2(0, -100), sf::Color::White)
    };

    window.draw(line, 4, sf::Lines);
}