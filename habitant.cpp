#include <cmath>
#include <functional>
#include <iostream>
#include <random>
#include <set>
#include <vector>
#include <algorithm>

class habitant{
    public:
        int x;
        int y;
};


class Island {
public:

    // ARGS
    std::function<double(habitant)> heuristic_function_ptr;
    int heuristic_id;

    // KWARGS
    int island_size;

    std::vector<habitant> habitants;
    std::vector<habitant> immigrants;

    Island(std::function<double(habitant)> heuristic_function, int heuristic_num, int size = 10)
        : heuristic_function_ptr(heuristic_function),
          heuristic_id(heuristic_num),
          island_size(size){}
        

    void initial_generation() {
        habitants.clear();
        habitants.reserve(island_size);

        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(-100, 100);

        for (int i = 0; i < island_size; i++) {
            habitants.push_back({ dist(gen), dist(gen) });
        }
    }

    
    void generate_next_generation(){

        // Random number generators
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> crossover_ratio(0.0, 1.0);
        std::normal_distribution<> mutation(0.0, 1.0);


        // *******************************
        // Evaluate fitness (keep top 50%)
        // *******************************

        // Calculate fitness by heuristic
        std::vector<std::pair<double, habitant>> fitness;
        for (auto &h : habitants) {
            fitness.push_back({heuristic(h), h});
        }

        // Sort by fitness (higher is better)
        std::sort(fitness.begin(), fitness.end(),
                [](auto &a, auto &b) { return a.first > b.first; });

        // Select top 25% as parents
        int num_parents = fitness.size() / 4;
        std::vector<habitant> parents;
        for (int i = 0; i < num_parents; ++i)
            parents.push_back(fitness[i].second);
        
        // Next 50% of island population consists of randomly sampling bottom 75% of island
        std::set<int> added;
        std::uniform_int_distribution<> bottom_three_quarters(num_parents, fitness.size() - 1);
        while(parents.size() < num_parents * 3){
            int index = bottom_three_quarters(gen);
            if (added.find(index) == added.end()){  
                added.insert(index);           
                parents.push_back(fitness[index].second);
            }
        }

        // ********************
        // Integrate Immigrants
        // ********************

        // Integrate missing 25% from immigrants
        parents.insert(parents.end(), immigrants.begin(), immigrants.end());
        
        // ******************
        // Crossover & Mutate
        // ******************

        // Randomly select parent from parent pool
        std::uniform_int_distribution<> parent_dist(0, parents.size() - 1);

        // Establish new generation
        std::vector<habitant> new_generation;

        // Keep top N original generation unchanged
        int elites = habitants.size() * 0.1;
        for (int i = 0; i < elites; ++i)
            new_generation.push_back(fitness[i].second);

        // Generate new offspring 
        while ((int)new_generation.size() < (int)habitants.size()) {
            habitant parent1 = parents[parent_dist(gen)];
            habitant parent2 = parents[parent_dist(gen)];

            // Simple arithmetic crossover (TODO: Make more complex)
            double alpha = crossover_ratio(gen);
            habitant child;
            child.x = static_cast<int>(alpha * parent1.x + (1 - alpha) * parent2.x);
            child.y = static_cast<int>(alpha * parent1.y + (1 - alpha) * parent2.y);

            // Small mutation (10% chance)
            if (crossover_ratio(gen) < 0.1) {
                child.x += static_cast<int>(mutation(gen));
                child.y += static_cast<int>(mutation(gen));
            }

            new_generation.push_back(child);
        }

        // Set new generation of inhabitants
        habitants = std::move(new_generation);
    }

    std::vector<const habitant*> select_immigrants(size_t M) {
        struct Scored {
            double score;
            size_t index;
        };

        std::vector<Scored> scores;
        scores.reserve(habitants.size());

        // Creates 
        for (size_t i = 0; i < habitants.size(); ++i)
            scores.push_back({ heuristic(habitants[i]), i });

        // Sort descending by fitness
        std::sort(scores.begin(), scores.end(),
                [](const auto& a, const auto& b){ return a.score > b.score; });

        // Pick top M as immigrants
        std::vector<const habitant*> immigrants;
        immigrants.reserve(M);
        for (size_t i = 0; i < M && i < scores.size(); ++i)
            immigrants.push_back(&habitants[scores[i].index]);

        return immigrants;
    }

    void receive_immigrants(std::vector<habitant> immigrant_list){
        immigrants = immigrant_list;
    }

private:
    double heuristic(habitant a) const {
        return heuristic_function_ptr(a);
    }
};