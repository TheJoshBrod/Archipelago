#include <cmath>
#include <functional>
#include <iostream>
#include <random>
#include <set>

class habitant{
    public:
        int x;
        int y;
};


class Island {
public:
    int id;
    int num_proc;
    std::function<double(habitant)> heuristic_function_ptr;
    std::vector<habitant> habitants;
    std::vector<habitant> immigrants;

    Island(std::function<double(habitant)> heuristic_function, int identity = -1, int p = -1)
        : heuristic_function_ptr(heuristic_function),
          id(identity == -1 ? generate_random_id() : identity),
          num_proc(p == -1 ? 1 : p) {}
    
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
        
        // Randomly select another 25% of bottom 75%
        std::set<int> added;
        std::uniform_int_distribution<> bottom_three_quarters(num_parents, fitness.size() - 1);
        while(parents.size() < num_parents * 2){
            int index = bottom_three_quarters(gen);
            if (added.find(index) == added.end()){  
                added.insert(index);           
                parents.push_back(fitness[index].second);
            }
        }

        // ********************
        // Integrate Immigrants
        // ********************

        parents.insert(parents.end(), immigrants.begin(), immigrants.end());
        
        // ******************
        // Crossover & Mutate
        // ******************

        // Randomly select parent from parent pool
        std::uniform_int_distribution<> parent_dist(0, parents.size() - 1);

        // Generate offspring
        std::vector<habitant> new_generation;
        while ((int)new_generation.size() < (int)habitants.size()) {
            habitant parent1 = parents[parent_dist(gen)];
            habitant parent2 = parents[parent_dist(gen)];

            // Simple arithmetic crossover
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

    }

private:
    static int generate_random_id() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(1, 100);
        return dist(gen);
    }

    double heuristic(habitant a) const {
        return heuristic_function_ptr(a);
    }
};


int main(int argc, char** argv){
    printf("Works!");
}