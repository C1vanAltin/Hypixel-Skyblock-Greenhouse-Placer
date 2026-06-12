#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <random>
#include <ctime>
using namespace std;
struct Slot {
    bool isCrop;
    bool mutSlot;
    bool helpsMutate;
    string name;
    vector<string> reqsForMutation;
};
Slot plot[10][10];
vector<std::string> Crops = {"Wheat","Carrot","Potato","Pumpkin","Sugar Cane","Melon","Cactus","Cocoa Beans","Red Mushroom","Brown Mushroom","Nether Wart",
 "Sunflower","Moonflower","Wild Rose","Fire","Dead Plant","Fermento","Ashwreath","Choconut","Dustgrain","Gloomgourd","Lonelilly","Scourroot","Shadevine",
 "Veilshroom","Witherbloom","Chocoberry","Cindershade","Coalroot","Creambloom","Duskbloom","Thornshade"};
struct MutationDef {
    string name;
    vector<string> reqs;
};
vector<MutationDef> DefinedMutations;
struct Gen {
    string genes[10][10];
    int fitness;
};
void initPlot() {
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++) {
            plot[i][j].isCrop = false;
            plot[i][j].mutSlot = false;
            plot[i][j].name = "";
            plot[i][j].helpsMutate = false;
            plot[i][j].reqsForMutation.clear();
        }
}

void plant(string name,int i,int j) {
    plot[i][j].isCrop = true;
    plot[i][j].mutSlot = false;
    plot[i][j].helpsMutate = false;
    plot[i][j].name = name;
    plot[i][j].reqsForMutation.clear();
}

void setAsSpreadSlot(string Mutname,int i,int j) {
    plot[i][j].isCrop = false;
    plot[i][j].mutSlot = true;
    plot[i][j].name = Mutname;
    plot[i][j].helpsMutate = false;
    plot[i][j].reqsForMutation.clear();

    for (const auto& m : DefinedMutations) {
        if (m.name == Mutname) {
            plot[i][j].reqsForMutation = m.reqs;
            break;
        }
    }
}

bool SpreadCheck(int r, int c) {
    // Belirtilen koordinattaki mutasyonun etrafındaki komşular tarafından karşılanıp karşılanmadığını kontrol eder
    if (!plot[r][c].mutSlot) return false;

    struct NeighborInfo { int row; int col; string name; };
    vector<NeighborInfo> neighbors;

    int dRow[] = {-1, 1, 0, 0, -1, -1, 1, 1};
    int dCol[] = {0, 0, -1, 1, -1, 1, -1, 1};

    for (int k = 0; k < 8; k++) {
        int newRow = r + dRow[k];
        int newCol = c + dCol[k];

        if (newRow >= 0 && newRow < 10 && newCol >= 0 && newCol < 10) {
            if (plot[newRow][newCol].isCrop) {
                neighbors.push_back({newRow, newCol, plot[newRow][newCol].name});
            }
        }
    }

    for (const string& requiredCrop : plot[r][c].reqsForMutation) {
        bool found = false;

        for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
            if (it->name == requiredCrop) {
                plot[it->row][it->col].helpsMutate = true;
                neighbors.erase(it);// birden fazla aynı crop için aynı crop u saymamak için
                found = true;
                break;
            }
        }

        if (!found) return false;
    }

    return true;
}

int FitnessFunction() {//İleride birden fazla denetince hangi kombinasyonun en iyisi olduğu belirleyecek
    int totalScore = 0;
    int activeMutations = 0;
    int totalMutSlots = 0;

    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++)
            plot[i][j].helpsMutate = false;

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (plot[i][j].mutSlot) {
                totalMutSlots++;

                if (SpreadCheck(i, j)) {
                    totalScore += 1000;// Başarılı mutasyona ödül
                    activeMutations++;
                }
            }
        }
    }

    if (activeMutations == 0) {
        return -5000;
    }

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (plot[i][j].isCrop) {
                if (plot[i][j].helpsMutate) {
                    totalScore += 50;// Mutasyona yarayan bitki ödülü
                } else {
                    totalScore -= 20;// Gereksiz bitki cezası
                }
            }
        }
    }

    int inactiveMutSlots = totalMutSlots - activeMutations;
    totalScore -= (inactiveMutSlots * 50);

    return totalScore;
}

void savePlotToFile(const string& filename) {
    ofstream outFile(filename);
    if (!outFile) return;

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            outFile << plot[i][j].isCrop << " "
                    << plot[i][j].mutSlot << " "
                    << (plot[i][j].name.empty() ? "None" : plot[i][j].name) << " "
                    << plot[i][j].reqsForMutation.size();

            for (const string& req : plot[i][j].reqsForMutation) {
                outFile << " " << req;
            }

            outFile << "\n";
        }
    }

    outFile.close();
}

void loadPlotFromFile(const string& filename) {
    ifstream inFile(filename);
    if (!inFile) return;

    string line;

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (!getline(inFile, line)) break;

            stringstream ss(line);
            string cropName;
            size_t reqSize;

            ss >> plot[i][j].isCrop >> plot[i][j].mutSlot >> cropName >> reqSize;

            plot[i][j].name = (cropName == "None") ? "" : cropName;
            plot[i][j].helpsMutate = false;
            plot[i][j].reqsForMutation.clear();

            for (size_t k = 0; k < reqSize; k++) {
                string reqName;
                ss >> reqName;
                plot[i][j].reqsForMutation.push_back(reqName);
            }
        }
    }

    inFile.close();
}

bool isMutationName(const string& name) {
    for (const auto& m : DefinedMutations) {
        if (m.name == name) return true;
    }
    return false;
}

string getRandomGene() {
    if (rand() % 100 < 25) {
        return "";
    }


    vector<string> validPool;
    for (MutationDef& m : DefinedMutations) {
        validPool.push_back(m.name);
        for (string& r : m.reqs) {
            validPool.push_back(r);
        }
    }

    if (validPool.empty()) {
        int totalSize = (int)(Crops.size() + DefinedMutations.size());
        int r = rand() % totalSize;
        if (r < (int)Crops.size()) return Crops[r];
        else return DefinedMutations[r - (int)Crops.size()].name;
    }

    return validPool[rand() % validPool.size()];
}

Gen createRandomIndividual() {
    Gen ind;
    ind.fitness = 0;

    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++)
            ind.genes[i][j] = getRandomGene();

    return ind;
}

int evaluate(Gen& a) {
    initPlot();

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (a.genes[i][j].empty()) continue;

            if (isMutationName(a.genes[i][j])) {
                setAsSpreadSlot(a.genes[i][j], i, j);
            } else {
                plant(a.genes[i][j], i, j);
            }
        }
    }

    a.fitness = FitnessFunction();
    return a.fitness;
}

Gen BestInGen(vector<Gen>& population) {
    Gen best = population[rand() % population.size()];

    for (int i = 0; i < 4; i++) {
        Gen challenger = population[rand() % population.size()];

        if (challenger.fitness > best.fitness)
            best = challenger;
    }

    return best;
}

Gen crossover(Gen& p1, Gen& p2) {
    Gen child;
    child.fitness = 0;

    int cut = rand() % 100;
    int counter = 0;

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (counter < cut)
                child.genes[i][j] = p1.genes[i][j];
            else
                child.genes[i][j] = p2.genes[i][j];

            counter++;
        }
    }

    return child;
}

void nextGen(Gen& ind) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (rand() % 1000 < 20) {
                ind.genes[i][j] = getRandomGene();
            }
        }
    }
}

Gen GeneticAlgorithm(int populationSize,int generations) {
    vector<Gen> population;

    for (int i = 0; i < populationSize; i++) {
        Gen ind = createRandomIndividual();
        evaluate(ind);
        population.push_back(ind);
    }

    for (int gen = 0; gen < generations; gen++) {
        sort(population.begin(), population.end(),
            [](const Gen& a, const Gen& b) {
                return a.fitness > b.fitness;
            });

        cout << "Generation " << gen
             << " Best Fitness = "
             << population[0].fitness << endl;

        vector<Gen> newPopulation;

        newPopulation.push_back(population[0]);
        newPopulation.push_back(population[1]);

        while ((int)newPopulation.size() < populationSize) {
            Gen parent1 = BestInGen(population);
            Gen parent2 = BestInGen(population);

            Gen child = crossover(parent1, parent2);
            nextGen(child);
            evaluate(child);

            newPopulation.push_back(child);
        }

        population = newPopulation;
    }

    sort(population.begin(), population.end(),
        [](const Gen& a, const Gen& b) {
            return a.fitness > b.fitness;
        });

    return population[0];
}

void saveMutationsToFile(const string& filename) {
    ofstream outFile(filename);
    if (!outFile) return;

    for (const auto& m : DefinedMutations) {
        outFile << m.name << "\n";
        outFile << m.reqs.size() << "\n";
        for (const string& r : m.reqs) {
            outFile << r << "\n";
        }
    }
    outFile.close();
}

void loadMutationsFromFile(const string& filename) {
    ifstream inFile(filename);
    if (!inFile) return;

    DefinedMutations.clear();
    string name;

    // Her döngüde ilk olarak mutasyon adını tüm satır olarak oku yoksa boşluklu croplar patlıo
    while (getline(inFile, name)) {
        if (name.empty()) continue;

        string sizeStr;
        if (!getline(inFile, sizeStr)) break;
        size_t size = stoi(sizeStr);

        vector<string> rList;
        for (size_t i = 0; i < size; i++) {
            string rName;
            getline(inFile, rName);
            rList.push_back(rName);
        }

        DefinedMutations.push_back({name, rList});
    }
    inFile.close();
}

int main() {
    srand((unsigned)time(0));
    string mutFile = "mutations.txt";
    loadMutationsFromFile(mutFile);
    int choice = 0;

    while (true) {
        cout << "HYPIXEL SKYBLOCK GREENHOUSE AUTO-OPTIMIZER" << endl;
        cout << "1. Define a mutation" << endl;
        cout << "2. List all mutations defined" << endl;
        cout << "3. Delete a mutation" << endl;
        cout << "4. List all Crops" << endl;
        cout << "5. Optimize the plot" << endl;
        cout << "6. Save and Exit" << endl;
        cout << "Choose your option: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid option." << endl;
            continue;
        }
        cin.ignore();

        if (choice == 1) {
            string mName;
            int reqCount;
            cout << "Mutation name: ";
            getline(cin, mName);
            cout << "Number of requirements: ";
            if (!(cin >> reqCount)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid number." << endl;
                continue;
            }
            cin.ignore();

            vector<string> tempReqs;

            for (int k = 0; k < reqCount; k++) {
                string rName;

                cout << k + 1 << ".Required Crop name: ";
                getline(cin, rName);

                bool validName = false;

                for (size_t x = 0; x < Crops.size(); x++) {
                    if (rName == Crops[x]) {
                        validName = true;
                        break;
                    }
                }

                if (validName) {
                    tempReqs.push_back(rName);
                } else {
                    cout << rName << " is invalid" << endl;
                }
            }

            DefinedMutations.push_back({mName, tempReqs});
            saveMutationsToFile(mutFile);
        }
        else if (choice == 2) {
            if (DefinedMutations.empty())
                cout << "No defined mutations." << endl;

            for (const auto& m : DefinedMutations) {
                cout << "- " << m.name << " -> ";
                for (const auto& r : m.reqs)
                    cout << r << " / ";
                cout << endl;
            }
        }
        else if (choice == 3) {
            if (DefinedMutations.empty()) {
                cout << "NO defined mutations to delete." << endl;
                continue;
            }

            string delName;
            cout << "Enter the full name of the mutation you want to delete: ";
            getline(cin, delName);

            auto it = remove_if(DefinedMutations.begin(), DefinedMutations.end(),
                [&delName](const MutationDef& m) {
                    return m.name == delName;
                });

            if (it != DefinedMutations.end()) {
                DefinedMutations.erase(it, DefinedMutations.end());
                saveMutationsToFile(mutFile);
                cout << delName << "deleted the mutation from the file." << endl;
            } else {
                cout << "NO such mutation is defined ." << endl;
            }
        }
        else if (choice == 4) {
            int a=1;
            for (string i : Crops) {

                cout <<a<<")"<<i << endl;
                a++;
            }
        }
        else if (choice == 5) {
            if (DefinedMutations.empty()) {
                cout << "Define something first" << endl;
                continue;
            }

            int popSize = 600;
            int genCount = 1200;
            int mutCount=0;
            Gen best = GeneticAlgorithm(popSize, genCount);
            evaluate(best);

            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 10; j++) {
                    if (plot[i][j].mutSlot) {
                        cout << "[" << plot[i][j].name << "]\t";
                    } else if (plot[i][j].isCrop) {
                        string flag = plot[i][j].helpsMutate ? "+" : "-";
                        cout << plot[i][j].name << "(" << flag << ")\t";
                    } else {
                        cout << "[ . ]\t";
                    }
                }
                cout << "\n\n";
            }
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 10; j++) {
                    if (plot[i][j].mutSlot) {
                        mutCount += 1;
                    }
                }
            }
            cout<<mutCount<<endl;
        }
        else if (choice == 6) {
            saveMutationsToFile(mutFile);
            break;
        }
        else {
            cout << "Invalid choice." << endl;
        }
    }
    return 0;
}