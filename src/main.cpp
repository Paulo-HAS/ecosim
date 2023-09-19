#define CROW_MAIN
#define CROW_STATIC_DIR "../public"

#include "crow_all.h"
#include "json.hpp"
#include <random>

static const uint32_t NUM_ROWS = 15;

// Constants
const uint32_t PLANT_MAXIMUM_AGE = 10;
const uint32_t HERBIVORE_MAXIMUM_AGE = 50;
const uint32_t CARNIVORE_MAXIMUM_AGE = 80;
const uint32_t MAXIMUM_ENERGY = 200;
const uint32_t THRESHOLD_ENERGY_FOR_REPRODUCTION = 20;
const uint32_t THRESHOLD_ENERGY_FOR_MOVING = 5;

// Probabilities
const double PLANT_REPRODUCTION_PROBABILITY = 0.2;
const double HERBIVORE_REPRODUCTION_PROBABILITY = 0.075;
const double CARNIVORE_REPRODUCTION_PROBABILITY = 0.025;
const double HERBIVORE_MOVE_PROBABILITY = 0.7;
const double HERBIVORE_EAT_PROBABILITY = 0.9;
const double CARNIVORE_MOVE_PROBABILITY = 0.5;
const double CARNIVORE_EAT_PROBABILITY = 1.0;

// Type definitions
enum entity_type_t
{
    empty,
    plant,
    herbivore,
    carnivore
};

struct pos_t
{
    uint32_t i;
    uint32_t j;
};

struct entity_t
{
    entity_type_t type;
    int32_t energy;
    int32_t age;
};

struct entity_info{
    entity_t* t;
    pos_t pos;
};

std::vector<entity_info> entity_vector; //vetor de entidades

// Auxiliary code to convert the entity_type_t enum to a string
NLOHMANN_JSON_SERIALIZE_ENUM(entity_type_t, {
                                                {empty, " "},
                                                {plant, "P"},
                                                {herbivore, "H"},
                                                {carnivore, "C"},
                                            })

// Auxiliary code to convert the entity_t struct to a JSON object
namespace nlohmann
{
    void to_json(nlohmann::json &j, const entity_t &e)
    {
        j = nlohmann::json{{"type", e.type}, {"energy", e.energy}, {"age", e.age}};
    }
}

// Grid that contains the entities
static std::vector<std::vector<entity_t>> entity_grid;


void spawn_entity(entity_type_t type){

    std::random_device rd;
    static std::mt19937 gen(rd());      //Generating random positions for the entities

    std::uniform_int_distribution<int> distribution_cols(0, NUM_ROWS-1);
    std::uniform_int_distribution<int> distribution_rows(0, NUM_ROWS-1);

    int random_col = distribution_cols(gen);
    int random_row = distribution_rows(gen);

    entity_t new_entity;
    new_entity.age = 0;
    new_entity.type = type;
    new_entity.energy = 100;
    
    entity_grid[random_row][random_col] = new_entity;

    entity_info info;
    info.t = &entity_grid[random_row][random_col];
    info.pos.i = random_row;
    info.pos.j = random_col;
    entity_vector.push_back(info);
}

void reproduction_sim(){
    std::random_device rd;
    static std::mt19937 gen(rd());      //Generating random chance for reproduction
    std::uniform_int_distribution<int> dice(0, 100);
    for(int it = 0; it < entity_vector.size(); it++){
        float chance = 100;
        int i = entity_vector[it].pos.i;
        int j = entity_vector[it].pos.j;

        bool cres = false;
        entity_type_t aux_type = entity_vector[it].t->type;
        if(aux_type == plant)
            chance *= PLANT_REPRODUCTION_PROBABILITY;
        else if(aux_type == herbivore)
            chance *= HERBIVORE_REPRODUCTION_PROBABILITY;
        else if(aux_type == carnivore)
            chance *= CARNIVORE_REPRODUCTION_PROBABILITY;
        
        if(i > 0 && entity_grid[i-1][j].type == empty){
            int rc = dice(gen);
            if(rc < chance){
                entity_t new_entity;
                new_entity.age = 0;
                new_entity.type = aux_type;
                new_entity.energy = 100;
                
                entity_grid[i - 1][j] = new_entity;

                entity_info info;
                info.t = &entity_grid[i - 1][j];
                info.pos.i = i-1;
                info.pos.j = j;
                entity_vector.push_back(info);
                cres = true;
            }
        }
        if(j > 0 && entity_grid[i][j-1].type == empty && cres == false){
            int rc = dice(gen);
            if(rc < chance){
                entity_t new_entity;
                new_entity.age = 0;
                new_entity.type = aux_type;
                new_entity.energy = 100;
                
                entity_grid[i][j-1] = new_entity;

                entity_info info;
                info.t = &entity_grid[i][j-1];
                info.pos.i = i;
                info.pos.j = j-1;
                entity_vector.push_back(info);
                cres = true;
            }
        }
        if(i < NUM_ROWS-1 && entity_grid[i+1][j].type == empty && cres == false){
            int rc = dice(gen);
            if(rc < chance){
                entity_t new_entity;
                new_entity.age = 0;
                new_entity.type = aux_type;
                new_entity.energy = 100;
                
                entity_grid[i+1][j] = new_entity;

                entity_info info;
                info.t = &entity_grid[i+1][j];
                info.pos.i = i+1;
                info.pos.j = j;
                entity_vector.push_back(info);
                cres = true;
            }
        }
        if(j < NUM_ROWS-1 && entity_grid[i][j+1].type == empty && cres == false){
            int rc = dice(gen);
            if(rc < chance){
                entity_t new_entity;
                new_entity.age = 0;
                new_entity.type = aux_type;
                new_entity.energy = 100;
                
                entity_grid[i][j+1] = new_entity;

                entity_info info;
                info.t = &entity_grid[i][j+1];
                info.pos.i = i;
                info.pos.j = j+1;
                entity_vector.push_back(info);
                cres = true;
            }
        }
    }

}

void kill_entity(int i, int j){
    for(int it = 0;it < entity_vector.size();i++){
        if(entity_vector[it].pos.i == i && entity_vector[it].pos.j == j){
            entity_vector.erase (entity_vector.begin() + it);
            entity_grid[i][j].type = empty;
            return;
        }
    }
}

void move_sim(){
    std::random_device rd;
    static std::mt19937 gen(rd());      //Generating random chance for moving
    std::uniform_int_distribution<int> dice(0, 100);
    
    for(int it = 0; it < entity_vector.size(); it++){
        float chance = 100;
        int i = entity_vector[it].pos.i;
        int j = entity_vector[it].pos.j;
        entity_type_t aux_type = entity_vector[it].t->type;
        bool moved = false;

        if(aux_type == herbivore)
            chance *= HERBIVORE_MOVE_PROBABILITY;
        else if(aux_type == carnivore)
            chance *= CARNIVORE_MOVE_PROBABILITY;


        if(aux_type == herbivore){
            if(i > 0 && entity_grid[i-1][j].type != carnivore){
                int mc = dice(gen);
                if(mc < chance){
                    if(entity_grid[i-1][j].type == plant){
                        entity_vector[it].t->energy += 30;
                        kill_entity(i-1,j);
                    }
                    entity_grid[i-1][j] = entity_grid[i][j];
                    entity_vector[it].t = &entity_grid[i-1][j];
                    entity_vector[it].pos.i--;
                    entity_grid[i][j].type = empty;
                    entity_vector[it].t->energy -= 5;
                }
            }
            if(j > 0 &&entity_grid[i][j-1].type != carnivore){
                int mc = dice(gen);
                if(mc < chance){
                    if(entity_grid[i][j-1].type == plant){
                        entity_vector[it].t->energy += 30;
                        kill_entity(i,j-1);
                    }
                    entity_grid[i][j-1] = entity_grid[i][j];
                    entity_vector[it].t = &entity_grid[i][j-1];
                    entity_vector[it].pos.j--;
                    entity_grid[i][j].type = empty;
                    entity_vector[it].t->energy -= 5;
                }
            }
            if(i < NUM_ROWS-1 && entity_grid[i+1][j].type != carnivore){
                int mc = dice(gen);
                if(mc < chance){
                    if(entity_grid[i+1][j].type == plant){
                        entity_vector[it].t->energy += 30;
                        kill_entity(i+1,j);
                    }
                    entity_grid[i+1][j] = entity_grid[i][j];
                    entity_vector[it].t = &entity_grid[i+1][j];
                    entity_vector[it].pos.i++;
                    entity_grid[i][j].type = empty;
                    entity_vector[it].t->energy -= 5;
                }
            }
            if(j < NUM_ROWS-1 && entity_grid[i][j+1].type != carnivore){
                int mc = dice(gen);
                if(mc < chance){
                    if(entity_grid[i][j+1].type == plant){
                        entity_vector[it].t->energy += 30;
                        kill_entity(i,j+1);
                    }
                    entity_grid[i][j+1] = entity_grid[i][j];
                    entity_vector[it].t = &entity_grid[i][j+1];
                    entity_vector[it].pos.j++;
                    entity_grid[i][j].type = empty;
                    entity_vector[it].t->energy -= 5;
                }
            }
        }

        if(aux_type == carnivore){
            if(i > 0 && entity_grid[i-1][j].type != plant){
                int mc = dice(gen);
                if(mc < chance){
                    if(entity_grid[i-1][j].type == herbivore){
                        entity_vector[it].t->energy += 20;
                        kill_entity(i-1,j);
                    }
                    entity_grid[i-1][j] = entity_grid[i][j];
                    entity_vector[it].t = &entity_grid[i-1][j];
                    entity_vector[it].pos.i--;
                    entity_grid[i][j].type = empty;
                    entity_vector[it].t->energy -= 5;
                }
            }
            if(j > 0 &&entity_grid[i][j-1].type != plant){
                int mc = dice(gen);
                if(mc < chance){
                    if(entity_grid[i][j-1].type == herbivore){
                        entity_vector[it].t->energy += 20;
                        kill_entity(i,j-1);
                    }
                    entity_grid[i][j-1] = entity_grid[i][j];
                    entity_vector[it].t = &entity_grid[i][j-1];
                    entity_vector[it].pos.j--;
                    entity_grid[i][j].type = empty;
                    entity_vector[it].t->energy -= 5;
                }
            }
            if(i < NUM_ROWS-1 && entity_grid[i+1][j].type != plant){
                int mc = dice(gen);
                if(mc < chance){
                    if(entity_grid[i+1][j].type == herbivore){
                        entity_vector[it].t->energy += 20;
                        kill_entity(i+1,j);
                    }
                    entity_grid[i+1][j] = entity_grid[i][j];
                    entity_vector[it].t = &entity_grid[i+1][j];
                    entity_vector[it].pos.i++;
                    entity_grid[i][j].type = empty;
                    entity_vector[it].t->energy -= 5;
                }
            }
            if(j < NUM_ROWS-1 && entity_grid[i][j+1].type != plant){
                int mc = dice(gen);
                if(mc < chance){
                    if(entity_grid[i][j+1].type == herbivore){
                        entity_vector[it].t->energy += 20;
                        kill_entity(i,j+1);
                    }
                    entity_grid[i][j+1] = entity_grid[i][j];
                    entity_vector[it].t = &entity_grid[i][j+1];
                    entity_vector[it].pos.j++;
                    entity_grid[i][j].type = empty;
                    entity_vector[it].t->energy -= 5;
                }
            }
        }
    }
}

void feed_sim(){
    std::random_device rd;
    static std::mt19937 gen(rd());      //Generating random chance for moving
    std::uniform_int_distribution<int> dice(0, 100);
    
    for(int it = 0; it < entity_vector.size(); it++){
        float chance = 100;
        int i = entity_vector[it].pos.i;
        int j = entity_vector[it].pos.j;
        entity_type_t aux_type = entity_vector[it].t->type;
        bool fed = false;
        if(aux_type == herbivore)
            chance *= HERBIVORE_EAT_PROBABILITY;
        else if(aux_type == carnivore)
            chance *= CARNIVORE_EAT_PROBABILITY;
        
        if(aux_type == herbivore){
            if(i > 0 && entity_grid[i-1][j].type == plant){
                int fc = dice(gen);
                if(fc > chance){
                    entity_vector[it].t->energy += 30;
                    kill_entity(i-1,j);
                }
            }
            if(j > 0 && entity_grid[i][j-1].type == plant){
                int fc = dice(gen);
                if(fc > chance){
                    entity_vector[it].t->energy += 30;
                    kill_entity(i,j-1);
                }
            }
            if(i < NUM_ROWS-1  && entity_grid[i+1][j].type == plant){
                int fc = dice(gen);
                if(fc > chance){
                    entity_vector[it].t->energy += 30;
                    kill_entity(i+1,j);
                }
            }
            if(j < NUM_ROWS-1 && entity_grid[i][j+1].type == plant){
                int fc = dice(gen);
                if(fc > chance){
                    entity_vector[it].t->energy += 30;
                    kill_entity(i,j+1);
                }
            }
        }
        if(aux_type == carnivore){
            if(i > 0 && entity_grid[i-1][j].type == herbivore){
                int fc = dice(gen);
                if(fc > chance){
                    entity_vector[it].t->energy += 20;
                    kill_entity(i-1,j);
                }
            }
            if(j > 0 && entity_grid[i][j-1].type == herbivore){
                int fc = dice(gen);
                if(fc > chance){
                    entity_vector[it].t->energy += 20;
                    kill_entity(i,j-1);
                }
            }
            if(i < NUM_ROWS-1  && entity_grid[i+1][j].type == herbivore){
                int fc = dice(gen);
                if(fc > chance){
                    entity_vector[it].t->energy += 20;
                    kill_entity(i+1,j);
                }
            }
            if(j < NUM_ROWS-1 && entity_grid[i][j+1].type == herbivore){
                int fc = dice(gen);
                if(fc > chance){
                    entity_vector[it].t->energy += 20;
                    kill_entity(i,j+1);
                }
            }
        }
    }
}

void death_sim(){
    for(int it = 0; it < entity_vector.size(); it++){
        int i = entity_vector[it].pos.i;
        int j = entity_vector[it].pos.j;
        if(entity_vector[it].t->energy <= 0)
            kill_entity(i,j);
        if(entity_vector[it].t->type == plant && (entity_vector[it].t->age >= PLANT_MAXIMUM_AGE || entity_vector[it].t->energy <= 0))
            kill_entity(i,j);
        if(entity_vector[it].t->type == herbivore && (entity_vector[it].t->age >= HERBIVORE_MAXIMUM_AGE || entity_vector[it].t->energy <= 0))
            kill_entity(i,j);
        if(entity_vector[it].t->type == carnivore && (entity_vector[it].t->age >= CARNIVORE_MAXIMUM_AGE || entity_vector[it].t->energy <= 0))
            kill_entity(i,j);
    }
}

void iterate_sim(){
    death_sim();
    feed_sim();
    move_sim();
    
    for(int i = 0; i < entity_vector.size(); i++)
        entity_vector[i].t->age++;
}

int main()
{
    crow::SimpleApp app;

    // Endpoint to serve the HTML page
    CROW_ROUTE(app, "/")
    ([](crow::request &, crow::response &res)
     {
        // Return the HTML content here
        res.set_static_file_info_unsafe("../public/index.html");
        res.end(); });

    CROW_ROUTE(app, "/start-simulation")
        .methods("POST"_method)([](crow::request &req, crow::response &res)
                                { 
        // Parse the JSON request body
        nlohmann::json request_body = nlohmann::json::parse(req.body);

       // Validate the request body 
        uint32_t total_entinties = (uint32_t)request_body["plants"] + (uint32_t)request_body["herbivores"] + (uint32_t)request_body["carnivores"];
        if (total_entinties > NUM_ROWS * NUM_ROWS) {
        res.code = 400;
        res.body = "Too many entities";
        res.end();
        return;
        }

        // Clear the entity grid
        entity_grid.clear();
        entity_grid.assign(NUM_ROWS, std::vector<entity_t>(NUM_ROWS, { empty, 0, 0}));
        
        // Create the entities
        // <YOUR CODE HERE>
                /////Ver com o professor como obter os valores iniciais do menu
        for(int i=0; i<(uint32_t)request_body["plants"];i++)
            spawn_entity(plant);
        for(int i=0; i<(uint32_t)request_body["carnivores"];i++)
            spawn_entity(carnivore);
        for(int i=0; i<(uint32_t)request_body["herbivores"];i++)
            spawn_entity(herbivore);


        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        res.body = json_grid.dump();
        res.end(); });

    // Endpoint to process HTTP GET requests for the next simulation iteration
    CROW_ROUTE(app, "/next-iteration")
        .methods("GET"_method)([]()
                               {
        // Simulate the next iteration
        // Iterate over the entity grid and simulate the behaviour of each entity
        
        // <YOUR CODE HERE>
        iterate_sim();
        
        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        return json_grid.dump(); });
    app.port(8080).run();

    return 0;
}