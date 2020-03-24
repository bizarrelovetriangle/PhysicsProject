#include "array_extentions.h"
#include <fstream>  
#include "gjk_result.h"
#include <list>
#include "minkowski_differens.h"
#include "minkowski_edge_distance.h"

class gjk_functions {
public:
	// todo: use separate class to draw lines and text
	// remove primitives

	template<size_t N, size_t M>
	static void EPA(
		primitives::vector2(&a_vectors)[N],
		primitives::vector2(&b_vectors)[M], 
		gjk_result gjk_result, sf::RenderWindow& window)
	{
		if (!gjk_result.is_collide) {
			return;
		}
		
		primitives::vector2 zero_vector(0);

		auto& mink_a = gjk_result.mink_a;
		auto& mink_b = gjk_result.mink_b;
		auto& mink_c = gjk_result.mink_c;

		draw_line(mink_a.differens, mink_b.differens, window, sf::Color::Cyan);
		draw_line(mink_b.differens, mink_c.differens, window, sf::Color::Cyan);
		draw_line(mink_c.differens, mink_a.differens, window, sf::Color::Cyan);

		double a_b_distance_o = line_point_distance(mink_a.differens, mink_b.differens, zero_vector);
		double a_c_distance_o = line_point_distance(mink_a.differens, mink_c.differens, zero_vector);
		double b_c_distance_o = line_point_distance(mink_b.differens, mink_c.differens, zero_vector);

		std::list<minkowski_edge_distance> edges_sort_by_distance;
		
		inseart_into_sorted_list(edges_sort_by_distance, minkowski_edge_distance(a_b_distance_o, mink_a, mink_b));
		inseart_into_sorted_list(edges_sort_by_distance, minkowski_edge_distance(a_c_distance_o, mink_a, mink_c));
		inseart_into_sorted_list(edges_sort_by_distance, minkowski_edge_distance(b_c_distance_o, mink_b, mink_c));

		for (int i = 0; i < 10; i++) {
			auto nearest_edge = *edges_sort_by_distance.begin();
			edges_sort_by_distance.pop_front();

			auto& nearest_mink_a = nearest_edge.mink_a;
			auto& nearest_mink_b = nearest_edge.mink_b;

			draw_line(nearest_mink_a.differens, nearest_mink_b.differens, window, sf::Color::Magenta);

			auto perpendicular_from_zero = perpendicular_from_point(nearest_mink_a.differens, nearest_mink_b.differens, zero_vector);

			auto new_mink_point = support_function(a_vectors, b_vectors, perpendicular_from_zero, window, 1, true);

			if (nearest_mink_a == new_mink_point || nearest_mink_b == new_mink_point)
			{
				draw_line(nearest_mink_a.differens, nearest_mink_b.differens, window, sf::Color::Blue);

				if (nearest_mink_a.point_a == nearest_mink_b.point_a) {
					draw_line(*nearest_mink_a.point_b, *nearest_mink_b.point_b, window, sf::Color::Blue);
				}
				else {
					draw_line(*nearest_mink_a.point_a, *nearest_mink_b.point_a, window, sf::Color::Blue);
				}

				return;
			}

			double new_mink_point_nearest_a_distance_o =
				line_point_distance(new_mink_point.differens, nearest_mink_a.differens, zero_vector);
			double new_mink_point_nearest_b_distance_o =
				line_point_distance(new_mink_point.differens, nearest_mink_b.differens, zero_vector);

			inseart_into_sorted_list(edges_sort_by_distance,
				minkowski_edge_distance(new_mink_point_nearest_a_distance_o, new_mink_point, nearest_mink_a));
			inseart_into_sorted_list(edges_sort_by_distance,
				minkowski_edge_distance(new_mink_point_nearest_b_distance_o, new_mink_point, nearest_mink_b));

			draw_line(new_mink_point.differens, nearest_mink_a.differens, window, sf::Color::Yellow);
			draw_line(new_mink_point.differens, nearest_mink_b.differens, window, sf::Color::Yellow);
		}
	}

	static void inseart_into_sorted_list(
		std::list<minkowski_edge_distance>& edges_sort_by_distance,
		minkowski_edge_distance new_edge_distance)
	{
		if (edges_sort_by_distance.empty() ||
			new_edge_distance.distance > edges_sort_by_distance.back().distance) 
		{
			edges_sort_by_distance.push_back(new_edge_distance);
			return;
		}

		auto position = std::find_if(edges_sort_by_distance.begin(), edges_sort_by_distance.end(),
			[&new_edge_distance](auto edge_distance)
			{
				return new_edge_distance.distance < edge_distance.distance;
			});

		edges_sort_by_distance.emplace(position, new_edge_distance);
	}

	template<size_t N, size_t M>
	static gjk_result GJK(
		primitives::vector2(&a_vectors)[N],
		primitives::vector2(&b_vectors)[M],
		sf::RenderWindow& window)
	{
		int counter = 1;

		primitives::vector2 zero_vector(0);
		primitives::vector2 direction(1, 0);

		auto mink_a = support_function(a_vectors, b_vectors, direction, window, 1, false);
		auto mink_b = support_function(a_vectors, b_vectors, -direction, window, 2, false);
		//draw_line(mink_a.differens, mink_b.differens, window, sf::Color::Red);
		//draw_number(mink_a.differens, window, 1);
		//draw_number(mink_b.differens, window, 2);

		for (int i = 0; i < 10; i++) {
			direction = perpendicular_to_point(mink_a.differens, mink_b.differens, zero_vector);
			auto mink_c = support_function(a_vectors, b_vectors, direction, window, i + 3, false);

			if (mink_a == mink_c || mink_b == mink_c) {
				return false;
			}

			//draw_line(b, c, window, sf::Color::Red);
			//draw_number(c, window, 3);

			if (triangle_contains(mink_a.differens, mink_b.differens, mink_c.differens, zero_vector)) {
				//draw_line(a, b, window, sf::Color::Cyan);
				//draw_line(b, c, window, sf::Color::Cyan);
				//draw_line(c, a, window, sf::Color::Cyan);
				return gjk_result(true, mink_a, mink_b, mink_c);
			}

			if (line_point_distance(mink_a.differens, mink_c.differens, zero_vector) <
				line_point_distance(mink_b.differens, mink_c.differens, zero_vector)) {
				//draw_line(a, c, window, sf::Color::Yellow);
				mink_b = mink_c;
			}
			else {
				//draw_line(b, c, window, sf::Color::Yellow);
				mink_a = mink_c;
			}
		}

		return gjk_result(false);;
	}

	template<size_t N, size_t M>
	static minkowski_differens support_function(
		primitives::vector2(&a_vectors)[N],
		primitives::vector2(&b_vectors)[M],
		primitives::vector2 direction,
		sf::RenderWindow& window, int number, bool draw_numder)
	{
		auto a = farthest_point(a_vectors, direction);
		auto b = farthest_point(b_vectors, -direction);
		if (draw_numder) {
			draw_number(*a, window, number);
			draw_number(*b, window, number);
		}

		return minkowski_differens(a, b, *a - *b);
	}

	template<size_t N>
	static bool contains_point(
		primitives::vector2 (&vectors) [N], 
		primitives::vector2 point,
		sf::RenderWindow& window)
	{
		primitives::vector2 direction(1, 0);

		auto a = *farthest_point(vectors, direction);
		auto b = *farthest_point(vectors, -direction);

		for (int i = 0; i < 10; i++) {
			direction = perpendicular_to_point(a, b, point);
			auto c = *farthest_point(vectors, direction);

			if (a == c || b == c) {
				return false;
			}

			if (triangle_contains(a, b, c, point)) {
				return true;
			}

			if (line_point_distance(a, c, point) < line_point_distance(b, c, point)) {
				b = c;
			}
			else {
				a = c;
			}
		}

		return false;
	}

	template<size_t N>
	static primitives::vector2* farthest_point(
		primitives::vector2(&vectors)[N], 
		primitives::vector2 direction = primitives::vector2()) 
	{
		auto* farthest_point = &vectors[0];
		auto farthest_dot_product = farthest_point->dot_product(direction);
		
		for (int i = 1; i < N; i++){
			auto dot_product = vectors[i].dot_product(direction);
	
			if (dot_product > farthest_dot_product) {
				farthest_dot_product = dot_product;
				farthest_point = &vectors[i];
			}
		}
	
		return farthest_point;
	}

	static primitives::vector2& perpendicular_to_point(
		primitives::vector2& a, primitives::vector2& b,
		primitives::vector2& o)
	{
		auto b_a = b - a;
		auto o_a = o - a;

		// negative cross product means that point is clockwise relative b_a
		if (b_a.is_clockwise(o_a))
		{
			b_a.negate_it();
		}

		// this method always return non clockwise perpendicular
		primitives::vector2 perp_to_point(-b_a.y, b_a.x);
		return perp_to_point;
	}

	static primitives::vector2& perpendicular_from_point(
		primitives::vector2& a, primitives::vector2& b,
		primitives::vector2& o)
	{
		auto b_a = b - a;
		auto o_a = o - a;

		// negative cross product means that point is clockwise relative b_a
		if (!b_a.is_clockwise(o_a))
		{
			b_a.negate_it();
		}

		// this method always return non clockwise perpendicular
		primitives::vector2 perp_to_point(-b_a.y, b_a.x);
		return perp_to_point;
	}

	// for convex and non convex shape
	static double line_point_distance(
		primitives::vector2& a, primitives::vector2& b,
		primitives::vector2& o) 
	{
		auto b_a = b - a;
		auto b_a_normalize = b_a.normalize();
		auto o_a = o - a;

		double projection_length = b_a_normalize.dot_product(o_a);

		primitives::vector2 projection_point;

		if (projection_length < 0) {
			projection_point = a;
		}
		else if (projection_length > b_a.length()) {
			projection_point = b;
		}
		else {
			projection_point = (b_a_normalize * projection_length) + a;
		}

		double line_point_distanse = projection_point.distanse(o);

		return line_point_distanse;
	}

	static bool triangle_contains(
		primitives::vector2& a, primitives::vector2& b, primitives::vector2& c,
		primitives::vector2& o)
	{
		bool b_a_cw = b.is_clockwise(o, a);
		bool c_b_cw = c.is_clockwise(o, b);
		bool a_c_cw = a.is_clockwise(o, c);

		if ((b_a_cw && c_b_cw && a_c_cw) || 
			!(b_a_cw || c_b_cw || a_c_cw)) {
			return true;
		}

		return false;
	}

	static void draw_line(
		primitives::vector2& a, primitives::vector2& b,
		sf::RenderWindow& window, sf::Color color)
	{
		sf::Vertex line[2] =
		{
			sf::Vertex(a, color),
			sf::Vertex(b, color)
		};
		window.draw(line, 2, sf::Lines);
	}

	static void draw_number(
		primitives::vector2& pos, sf::RenderWindow& window, int number)
	{
		draw_text(pos, window, std::to_string(number));
	}

	static void draw_text(
		primitives::vector2& pos, sf::RenderWindow& window, std::string str)
	{
		static int i = 1;
		static sf::Font font;
		if (i == 1 && !font.loadFromFile("arial.ttf"))
		{
			throw "error?";
		}
		i = 0;

		sf::Text text;
		text.setFont(font);
		text.setString(str);
		text.setPosition(pos);
		text.setFillColor(sf::Color::Blue);
		text.setCharacterSize(20);
		window.draw(text);
	}
};