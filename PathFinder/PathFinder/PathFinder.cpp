#include "stdafx.h"

#include <stdlib.h>
#include <assert.h>

#include <iostream>
#include <string>
#include <iomanip>
#include <random>
#include <vector>
#include <set>
#include <map>

static constexpr int
WIDTH = 15,
HEIGHT = 10;

struct Node
{
    Node()
        : Cost( 0 ), IsTraversable( true )
    { }

    Node( int x, int y )
        : X( x ), Y( y ), Cost( 0 ), IsTraversable( true )
    { }

    bool operator==( const Node & other ) const
    {
        return X == other.X
            && Y == other.Y;
    }

    bool operator!=( const Node & other ) const
    {
        return !operator==( other );
    }

    bool operator<( const Node & other ) const
    {
        if ( Y != other.Y )
        {
            return Y < other.Y;
        }

        return X < other.X;
    }

    int X;
    int Y;
    int Cost;
    bool IsTraversable;
};

std::ostream & operator<<( std::ostream & os, const Node & point )
{
    return os << point.X << "-" << point.Y;
}

template< typename NODE_LAMBDA, typename ROW_LAMBDA >
void foreach_node( const NODE_LAMBDA & node_lambda, const ROW_LAMBDA & row_lambda )
{
    for ( auto y = 0; y < HEIGHT; y++ )
    {
        for ( auto x = 0; x < WIDTH; x++ )
        {
            node_lambda( x, y );
        }

        row_lambda();
    }
}

template< typename NODE_LAMBDA >
void foreach_node( const NODE_LAMBDA & node_lambda )
{
    for ( auto y = 0; y < HEIGHT; y++ )
    {
        for ( auto x = 0; x < WIDTH; x++ )
        {
            node_lambda( x, y );
        }
    }
}

int manhattan_distance( const Node & start, const Node & end )
{
    return std::abs( start.X - end.X ) + std::abs( start.Y - end.Y );
}

class AStar
{
public:
    
    AStar( const Node & start, const Node & end )
        : Start( start ), End( end )
    { 
        OpenSet.insert( start );

        const auto infinity = std::numeric_limits< int >::max();

        foreach_node( [this, infinity ] ( size_t x, size_t y )
        {
            Node node( x, y );

            FromStartNodeCostMap.insert( std::make_pair( node, infinity ) );
            ThroughNodeCostMap.insert( std::make_pair( node, infinity ) );
        } );

        FromStartNodeCostMap[ Start ] = 0;
        ThroughNodeCostMap[ Start ] = manhattan_distance( Start, End );
    }

    bool NextMove()
    {
        assert( !OpenSet.empty() );

        auto current = GetLowestThroughNodeCostFromOpenSet();

        OpenSet.erase( current );
        ClosedSet.insert( current );

        if ( current == End )
        {
            return true;
        }

        const auto neighbors = GetNeighbors( current );

        for ( const auto & neighbor : neighbors )
        {
            const auto closed_set_ite = ClosedSet.find( neighbor );

            if ( closed_set_ite != ClosedSet.cend() )
            {
                continue;
            }

            const auto distance_current_to_neighbor = manhattan_distance( current, neighbor );
            const auto tentative_gscore = ThroughNodeCostMap.at( current ) + distance_current_to_neighbor;

            const auto open_set_ite = OpenSet.find( neighbor );

            if ( open_set_ite == OpenSet.cend() )
            {
                OpenSet.insert( neighbor );
            }
            else if ( tentative_gscore >= ThroughNodeCostMap.at( neighbor ) )
            {
                continue;
            }

            CameFromMap.insert( std::make_pair( neighbor, current ) );
            ThroughNodeCostMap[ neighbor ] = tentative_gscore;
            FromStartNodeCostMap[ neighbor ] = tentative_gscore + manhattan_distance( neighbor, End );
        }

        return false;
    }

    Node GetLowestThroughNodeCostFromOpenSet() const
    {
        Node result;
        int max_cost = std::numeric_limits< int >::max();
        
        for ( const auto & node : OpenSet )
        {
            const auto cost = ThroughNodeCostMap.at( node );

            if ( cost < max_cost )
            {
                result = node;
                max_cost = cost;
            }
        }

        return result;
    }

    std::vector< Node > GetNeighbors( const Node & node ) const
    {
        std::vector< Node > neighbors;

        if ( node.X > 0 )
        {
            neighbors.push_back( Node( node.X - 1, node.Y ) );
        }
        if ( node.X > 0 && node.Y < HEIGHT )
        {
            neighbors.push_back( Node( node.X - 1, node.Y + 1 ) );
        }
        if ( node.Y < HEIGHT )
        {
            neighbors.push_back( Node( node.X, node.Y + 1 ) );
        }
        if ( node.X < WIDTH && node.Y < HEIGHT )
        {
            neighbors.push_back( Node( node.X + 1, node.Y + 1 ) );
        }
        if ( node.X < WIDTH )
        {
            neighbors.push_back( Node( node.X + 1, node.Y ) );
        }
        if ( node.X < WIDTH && node.Y > 0 )
        {
            neighbors.push_back( Node( node.X + 1, node.Y - 1 ) );
        }
        if ( node.Y > 0 )
        {
            neighbors.push_back( Node( node.X, node.Y - 1 ) );
        }
        if ( node.X > 0 && node.Y > 0 )
        {
            neighbors.push_back( Node( node.X - 1, node.Y - 1 ) );
        }

        return neighbors;
    }

    Node Start, End;
    std::set< Node > OpenSet, ClosedSet;
    std::map< Node, Node > CameFromMap;
    std::map< Node, int > FromStartNodeCostMap;
    std::map< Node, int > ThroughNodeCostMap;
};

int main()
{
    std::cout << "Hello" << std::endl;

    std::random_device rd;
    std::mt19937 mt( rd() );
    std::uniform_int_distribution<int> width_dist( 0, WIDTH - 1 );
    std::uniform_int_distribution<int> height_dist( 0, HEIGHT - 1 );

    Node start( width_dist( mt ), height_dist( mt ) );
    Node end;

    do 
    {
        end.X = width_dist( mt );
        end.Y = height_dist( mt );
    } while ( start == end );

    /*Node start( 5, 5 );
    Node end( 10, 5 );*/

    const auto print_intermediate_steps = false;

    system( "CLS" );

    std::cout << "Start : " << start << std::endl;
    std::cout << "End : " << end << std::endl;

    AStar a_star( start, end );

    while ( !a_star.NextMove() )
    {
        if ( print_intermediate_steps )
        {
            foreach_node(
                [ &start, &end, &a_star ] ( int x, int y )
            {
                Node point( x, y );

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
                    const int cost = a_star.FromStartNodeCostMap.at( point );

                    out = std::to_string( cost == std::numeric_limits< int >::max() ? 0 : cost );
                }

                std::cout << /*std::left <<*/ "[" << std::setw( 4 ) << out.c_str() << "]";
            },
                [] ()
            {
                std::cout << std::endl;
            }
            );

            int get = std::cin.get();
            system( "CLS" );
        }
    }

    Node from( -1, -1 );
    Node to = a_star.End;

    std::vector< Node > nodes;
    
    while ( from != a_star.Start )
    {
        from = a_star.CameFromMap.at( to );
        to = from;

        nodes.push_back( from );
    }

    foreach_node(
        [ &start, &end, &a_star, &nodes ] ( int x, int y )
    {
        Node point( x, y );

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
            const auto ite = std::find( nodes.cbegin(), nodes.cend(), point );

            if ( ite != nodes.cend() )
            {
                out = "X";
            }
            else
            {
                out = " ";
            }
        }

        std::cout << std::left << std::setw( 4 ) << "[" << out.c_str() << "]";
    },
        [] ()
    {
        std::cout << std::endl;
    }
    );

    int get = std::cin.get();

    return 0;
}

