﻿#include "stdafx.h"

#include <stdlib.h>
#include <assert.h>

#include <iostream>
#include <queue>
#include <string>
#include <iomanip>
#include <random>
#include <vector>
#include <set>
#include <map>

struct Point
{
    Point()
    { }

    Point( int x, int y )
        : X( x ), Y( y )
    { }

    bool operator==( const Point & other ) const
    {
        return X == other.X
            && Y == other.Y;
    }

    bool operator!=( const Point & other ) const
    {
        return !operator==( other );
    }

    bool operator<( const Point & other ) const
    {
        if ( Y != other.Y )
        {
            return Y < other.Y;
        }

        return X < other.X;
    }

    int X;
    int Y;
};

std::ostream & operator<<( std::ostream & os, const Point & point )
{
    return os << point.X << "-" << point.Y;
}

class ManahattanDistance
{
public:
    
    int operator()( const Point & start, const Point & end ) const
    {
        return std::abs( start.X - end.X ) + std::abs( start.Y - end.Y );
    }
};

class Graph
{
public:

    Graph( const int width, const int height )
        : Width( width ), Height( height )
    { }

    void SetObstacles( const std::set< Point > & obstacles )
    {
        Obstacles = obstacles;
    }

    void GetNeighbors( std::vector< Point > & neighbors, const Point & node ) const
    {
        for ( int x = -1; x <= 1; x++ )
        {
            for ( int y = -1; y <= 1; y++ )
            {
                if ( x == 0 && y == 0 )
                {
                    continue;
                }

                Point point( node.X + x, node.Y + y );
                
                if ( point.X < 0 
                     || point.X == Width
                     || point.Y < 0
                     || point.Y == Height
                     )
                {
                    continue;
                }

                if ( Obstacles.find( point ) != Obstacles.cend() )
                {
                    continue;
                }

                neighbors.emplace_back( point );
            }
        }
    }

    int GetCost( const Point & from, const Point & to ) const
    {
        return 1;
    }

private:

    int Width;
    int Height;
    std::set< Point > Obstacles;
};

struct PriorityPoint
{
    Point Point;
    int Priority;

    bool operator<( const PriorityPoint & other ) const
    {
        return Priority < other.Priority;
    }

    bool operator>( const PriorityPoint & other ) const
    {
        return Priority > other.Priority;
    }
};

void reconstruct_path( std::vector< Point > & path, const Point & start, const Point & end, const std::map< Point, Point > & came_from_map )
{
    Point current = end;

    while ( current != start )
    {
        current = came_from_map.at( current );
        path.push_back( current );
    }

    path.push_back( start );
}

template< typename HEURISTIC_FUNCTION >
void a_star_search( std::vector< Point > & path, const Graph & graph, const Point & start, const Point & end, HEURISTIC_FUNCTION heuristic )
{
    std::priority_queue< PriorityPoint, std::vector< PriorityPoint >, std::greater< PriorityPoint > > open_set;
    open_set.push( { start, 0 } );

    std::map<Point, Point> came_from_map;
    std::map<Point, int> from_start_node_cost_map;

    came_from_map[ start ] = start;
    from_start_node_cost_map[ start ] = 0;

    while ( !open_set.empty() )
    {
        auto current = open_set.top();

        if ( current.Point == end )
        {
            reconstruct_path( path, start, end, came_from_map );
            break;
        }

        open_set.pop();

        std::vector< Point > neighbors;
        graph.GetNeighbors( neighbors, current.Point );

        for ( const auto & neighbor : neighbors )
        {
            int new_cost = from_start_node_cost_map[ current.Point ] + graph.GetCost( current.Point, neighbor );

            if ( from_start_node_cost_map.find( neighbor ) == from_start_node_cost_map.end()
                 || new_cost < from_start_node_cost_map[ neighbor ]
                 )
            {
                from_start_node_cost_map[ neighbor ] = new_cost;
                auto priority = new_cost + heuristic( neighbor, end );
                open_set.push( { neighbor, priority } );
                came_from_map[ neighbor ] = current.Point;
            }
        }
    }
}

void draw_grid( const int width, const int height, const Point & start, const Point & end, const std::vector< Point > & path, const std::set< Point > & obstacles )
{
    std::cout << std::setw( 3 ) << " ";

    for ( auto x = 0; x < width; x++ )
    {
        std::cout << std::setw( 3 ) << std::to_string( x );
    }

    std::cout << std::endl;

    for ( auto y = 0; y < height; y++ )
    {
        std::cout << std::setw( 3 ) << std::to_string( y );

        for ( auto x = 0; x < width; x++ )
        {
            Point point( x, y );

            std::string out;

            if ( point == start )
            {
                out = "S";
            }
            else if ( point == end )
            {
                out = "E";
            }
            else
            {
                const auto ite = std::find( path.cbegin(), path.cend(), point );

                if ( ite != path.cend() )
                {
                    out = "O";
                }
                else if ( obstacles.find( point ) != obstacles.cend() )
                {
                    out = "X";
                }
                else
                {
                    out = ".";
                }
            }

            std::cout << std::setw( 3 ) << out.c_str();
        }

        std::cout << std::endl;
    }
}

int main()
{
    static constexpr int
        WIDTH = 30,
        HEIGHT = 20,
        OBSTACLES_COUNT = 50;

    std::random_device rd;
    std::mt19937 mt( rd() );
    std::uniform_int_distribution<int> width_dist( 0, WIDTH - 1 );
    std::uniform_int_distribution<int> height_dist( 0, HEIGHT - 1 );

    std::set< Point > obstacles;

    for ( int i = 0; i < OBSTACLES_COUNT; ++i )
    {
        Point obstacle;

        do
        {
            obstacle.X = width_dist( mt );
            obstacle.Y = height_dist( mt );
        } while ( obstacles.find( obstacle ) != obstacles.end() );

        obstacles.insert( obstacle );
    }

    Point start;
    Point end;

    do
    {
        start.X = width_dist( mt );
        start.Y = height_dist( mt );
    } while ( obstacles.find( start ) != obstacles.cend() );

    do 
    {
        end.X = width_dist( mt );
        end.Y = height_dist( mt );
    } while ( start == end || obstacles.find( end ) != obstacles.cend() );

    const auto print_intermediate_steps = false;

    system( "CLS" );

    std::cout << "Start : " << start << std::endl;
    std::cout << "End : " << end << std::endl;

    std::cout << std::endl << std::endl;

    Graph graph( WIDTH, HEIGHT );
    graph.SetObstacles( obstacles );

    std::vector< Point > path;
    a_star_search( path, graph, start, end, ManahattanDistance() );

    draw_grid( WIDTH, HEIGHT, start, end, path, obstacles );

    int get = std::cin.get();

    return 0;
}

