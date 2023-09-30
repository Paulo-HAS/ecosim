#define CROW_MAIN
#define CROW_STATIC_DIR "../public"

#include "crow_all.h"
#include "json.hpp"
#include <random>
#include <thread>

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

struct entity_info{     //struct com um ponteiro para a entidade e sua posição
    entity_t* t;
    pos_t pos;
};

bool g_clock = false;


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

void simulateEntity(entity_info* en);   //declaração antecipada da função de simulação para poder ser chamada na reprodução

void reproduction_sim(entity_info* en){     //reprodução dos indivíduos
    std::random_device rd;
    static std::mt19937 gen(rd());      //Generating random chance for reproduction
    std::uniform_int_distribution<int> dice(0, 100);
    float chance = 100;
    int i = en->pos.i;
    int j = en->pos.j;

    bool cres = false;
    entity_type_t aux_type = en->t->type;
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
            std::thread (simulateEntity, &info).detach();
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
            std::thread (simulateEntity, &info).detach();
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
            std::thread (simulateEntity, &info).detach();
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
            std::thread (simulateEntity, &info).detach();
            cres = true;
        }
    }

}

void kill_entity(entity_t* t){     //esvazia a posição no grid, o que fará o thread correspondente parar
    t->type = empty;
}

void move_sim(entity_info* en){     //movimentação dos animais
    std::random_device rd;
    static std::mt19937 gen(rd());      //Generating random chance for moving
    std::uniform_int_distribution<int> dice(0, 100);
    
    float chance = 100;
    int i = en->pos.i;
    int j = en->pos.j;
    entity_type_t aux_type = en->t->type;
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
                    en->t->energy += 30;
                    kill_entity(&entity_grid[i-1][j]);
                }
                entity_grid[i-1][j] = entity_grid[i][j];
                en->t = &entity_grid[i-1][j];
                en->pos.i--;
                entity_grid[i][j].type = empty;
                en->t->energy -= 5;
            }
        }
        if(j > 0 &&entity_grid[i][j-1].type != carnivore){
            int mc = dice(gen);
            if(mc < chance){
                if(entity_grid[i][j-1].type == plant){
                    en->t->energy += 30;
                    kill_entity(&entity_grid[i][j-1]);
                }
                entity_grid[i][j-1] = entity_grid[i][j];
                en->t = &entity_grid[i][j-1];
                en->pos.j--;
                entity_grid[i][j].type = empty;
                en->t->energy -= 5;
            }
        }
        if(i < NUM_ROWS-1 && entity_grid[i+1][j].type != carnivore){
            int mc = dice(gen);
            if(mc < chance){
                if(entity_grid[i+1][j].type == plant){
                    en->t->energy += 30;
                    kill_entity(&entity_grid[i+1][j]);
                }
                entity_grid[i+1][j] = entity_grid[i][j];
                en->t = &entity_grid[i+1][j];
                en->pos.i++;
                entity_grid[i][j].type = empty;
                en->t->energy -= 5;
            }
        }
        if(j < NUM_ROWS-1 && entity_grid[i][j+1].type != carnivore){
            int mc = dice(gen);
            if(mc < chance){
                if(entity_grid[i][j+1].type == plant){
                    en->t->energy += 30;
                    kill_entity(&entity_grid[i][j+1]);
                }
                entity_grid[i][j+1] = entity_grid[i][j];
                en->t = &entity_grid[i][j+1];
                en->pos.j++;
                entity_grid[i][j].type = empty;
                en->t->energy -= 5;
            }
        }
    }

    if(aux_type == carnivore){
        if(i > 0 && entity_grid[i-1][j].type != plant){
            int mc = dice(gen);
            if(mc < chance){
                if(entity_grid[i-1][j].type == herbivore){
                    en->t->energy += 20;
                    kill_entity(&entity_grid[i-1][j]);
                }
                entity_grid[i-1][j] = entity_grid[i][j];
                en->t = &entity_grid[i-1][j];
                en->pos.i--;
                entity_grid[i][j].type = empty;
                en->t->energy -= 5;
            }
        }
        if(j > 0 &&entity_grid[i][j-1].type != plant){
            int mc = dice(gen);
            if(mc < chance){
                if(entity_grid[i][j-1].type == herbivore){
                    en->t->energy += 20;
                    kill_entity(&entity_grid[i][j-1]);
                }
                entity_grid[i][j-1] = entity_grid[i][j];
                en->t = &entity_grid[i][j-1];
                en->pos.j--;
                entity_grid[i][j].type = empty;
                en->t->energy -= 5;
            }
        }
        if(i < NUM_ROWS-1 && entity_grid[i+1][j].type != plant){
            int mc = dice(gen);
            if(mc < chance){
                if(entity_grid[i+1][j].type == herbivore){
                    en->t->energy += 20;
                    kill_entity(&entity_grid[i+1][j]);
                }
                entity_grid[i+1][j] = entity_grid[i][j];
                en->t = &entity_grid[i+1][j];
                en->pos.i++;
                entity_grid[i][j].type = empty;
                en->t->energy -= 5;
            }
        }
        if(j < NUM_ROWS-1 && entity_grid[i][j+1].type != plant){
            int mc = dice(gen);
            if(mc < chance){
                if(entity_grid[i][j+1].type == herbivore){
                    en->t->energy += 20;
                    kill_entity(&entity_grid[i][j+1]);
                }
                entity_grid[i][j+1] = entity_grid[i][j];
                en->t = &entity_grid[i][j+1];
                en->pos.j++;
                entity_grid[i][j].type = empty;
                en->t->energy -= 5;
            }
        }
    }
}

void feed_sim(entity_info* en){     //alimentação dos animais
    std::random_device rd;
    static std::mt19937 gen(rd());      //Generating random chance for moving
    std::uniform_int_distribution<int> dice(0, 100);
    
    float chance = 100;
    int i = en->pos.i;
    int j = en->pos.j;
    entity_type_t aux_type = en->t->type;
    bool fed = false;
    if(aux_type == herbivore)
        chance *= HERBIVORE_EAT_PROBABILITY;
    else if(aux_type == carnivore)
        chance *= CARNIVORE_EAT_PROBABILITY;
    
    if(aux_type == herbivore){
        if(i > 0 && entity_grid[i-1][j].type == plant){
            int fc = dice(gen);
            if(fc > chance){
                en->t->energy += 30;
                kill_entity(&entity_grid[i-1][j]);
            }
        }
        if(j > 0 && entity_grid[i][j-1].type == plant){
            int fc = dice(gen);
            if(fc > chance){
                en->t->energy += 30;
                kill_entity(&entity_grid[i][j-1]);
            }
        }
        if(i < NUM_ROWS-1  && entity_grid[i+1][j].type == plant){
            int fc = dice(gen);
            if(fc > chance){
                en->t->energy += 30;
                kill_entity(&entity_grid[i+1][j]);
            }
        }
        if(j < NUM_ROWS-1 && entity_grid[i][j+1].type == plant){
            int fc = dice(gen);
            if(fc > chance){
                en->t->energy += 30;
                kill_entity(&entity_grid[i][j+1]);
            }
        }
    }
    if(aux_type == carnivore){
        if(i > 0 && entity_grid[i-1][j].type == herbivore){
            int fc = dice(gen);
            if(fc > chance){
                en->t->energy += 20;
                kill_entity(&entity_grid[i-1][j]);
            }
        }
        if(j > 0 && entity_grid[i][j-1].type == herbivore){
            int fc = dice(gen);
            if(fc > chance){
                en->t->energy += 20;
                kill_entity(&entity_grid[i][j-1]);
            }
        }
        if(i < NUM_ROWS-1  && entity_grid[i+1][j].type == herbivore){
            int fc = dice(gen);
            if(fc > chance){
                en->t->energy += 20;
                kill_entity(&entity_grid[i+1][j]);
            }
        }
        if(j < NUM_ROWS-1 && entity_grid[i][j+1].type == herbivore){
            int fc = dice(gen);
            if(fc > chance){
                en->t->energy += 20;
                kill_entity(&entity_grid[i][j+1]);
            }
        }
    }
}

bool death_sim(entity_info* en){       //verifica morte por idade/energia
    if(en->t->type == plant && (en->t->age >= PLANT_MAXIMUM_AGE || en->t->energy <= 0)){
        kill_entity(en->t);
        return false;
    }
    else if(en->t->type == herbivore && (en->t->age >= HERBIVORE_MAXIMUM_AGE || en->t->energy <= 0)){
        kill_entity(en->t);
        return false;
    }
    else if(en->t->type == carnivore && (en->t->age >= CARNIVORE_MAXIMUM_AGE || en->t->energy <= 0)){
        kill_entity(en->t);
        return false;
    }
    return true;
}

void simulateEntity(entity_info* en){   //função das threads
    bool clock = false;     //Se o clock individual das threads difere do global, é porque houve uma iteração
    bool alive = true;
    while(alive){
        if(en->t->type == empty)
            return;
        if(clock != g_clock){
            feed_sim(en);
            reproduction_sim(en);
            move_sim(en);
            en->t->age++;   //envelhece
            alive = death_sim(en);
        }
    }

}

void spawn_entity(entity_type_t type){      //gera os indivíduos iniciais

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
    std::thread (simulateEntity, &info).detach();
}

void iterate_sim(){     //função que roda o clock global de simulação acompanhando iterações
    if(g_clock)
            g_clock = false;
        else
            g_clock = true;
        printf("\nClock");
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