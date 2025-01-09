#include <iostream>
#include <memory>
#include <exception>
#include <fstream>
#include <map>
#include <string>
#include <vector>

// Define states for resource management
enum class ResourceState {
    Idle,
    InUse,
    UnderMaintenance
};

// Forward declaration of Project class to resolve cyclic dependency.
class Project;

// Resource class
class Resource {
public:
    Resource(const std::string& id, const std::string& type) : id(id), type(type), state(ResourceState::Idle), allocatedProject(nullptr) {}
    ~Resource() = default;

    std::string getId() const { return id; }
    const std::string& getType() const { return type; }
    ResourceState getState() const { return state; }
    void setState(ResourceState newState) { state = newState; }
    void allocateToProject(const std::shared_ptr<Project>& project) { allocatedProject = project; }
    std::shared_ptr<Project> getAllocatedProject() const { return allocatedProject; }

private:
    std::string id;
    std::string type;
    ResourceState state;
    std::shared_ptr<Project> allocatedProject; // Pointer to the project this resource is allocated to
};

// Project class
class Project : public std::enable_shared_from_this<Project> {
public:
    Project(const std::string& id, const std::string& name) : id(id), name(name) {}
    ~Project() = default;

    std::string getId() const { return id; }
    const std::string& getName() const { return name; }
    const std::vector<std::shared_ptr<Resource>>& getResources() const { return resources; }

    void addResource(const std::shared_ptr<Resource>& resource) {
        resources.push_back(resource);
        resource->allocateToProject(shared_from_this());
    }

private:
    std::string id;
    std::string name;
    std::vector<std::shared_ptr<Resource>> resources;
};

// ResourceManager class
class ResourceManager {
public:
    ResourceManager() = default;
    ~ResourceManager() = default;

    void addResource(const std::string& id, const std::string& type) {
        resources[id] = std::make_shared<Resource>(id, type);
    }

    std::shared_ptr<Resource> getResource(const std::string& id) {
        if (resources.find(id) != resources.end()) {
            return resources[id];
        } else {
            throw std::runtime_error("Resource not found");
        }
    }

    void logTransaction(const std::string& message) {
        std::ofstream logFile("resource_log.txt", std::ios_base::app);
        logFile << message << std::endl;
    }

    void allocateResourceToProject(const std::string& resourceId, const std::shared_ptr<Project>& project) {
        auto resource = getResource(resourceId);
        resource->setState(ResourceState::InUse);
        project->addResource(resource);
        logTransaction("Resource " + resourceId + " allocated to project " + project->getName());
    }

    void addProject(const std::string& id, const std::string& name) {
        projects[id] = std::make_shared<Project>(id, name);
    }

    std::shared_ptr<Project> getProject(const std::string& id) {
        if (projects.find(id) != projects.end()) {
            return projects[id];
        } else {
            throw std::runtime_error("Project not found");
        }
    }

    void displayResourceState(const std::string& id) {
        auto resource = getResource(id);
        std::cout << "Resource " << id << " is ";
        switch (resource->getState()) {
            case ResourceState::Idle:
                std::cout << "Idle";
                break;
            case ResourceState::InUse:
                std::cout << "In Use";
                break;
            case ResourceState::UnderMaintenance:
                std::cout << "under maintenance";
                auto project = resource->getAllocatedProject();
                if (project) {
                    std::cout << " and allocated to project " << project->getName();
                }
                break;
        }
        std::cout << ".\n";
    }

    void maintainResource(const std::string& id) {
        auto resource = getResource(id);
        if (resource->getType() == "equipment") {
            resource->setState(ResourceState::UnderMaintenance);
            logTransaction("Resource " + id + " is under maintenance.");
            std::cout << "Resource " << id << " is under maintenance.\n";
        } else {
            std::cout << "Resource " << id << " is not equipment and cannot be maintained.\n";
        }
    }

private:
    std::map<std::string, std::shared_ptr<Resource>> resources;
    std::map<std::string, std::shared_ptr<Project>> projects;
};

int main() {
    ResourceManager rm;
    int choice, typeChoice;
    std::string id, type, name;

    try {
        do {
            std::cout << "1. Add Resource\n2. Use Resource\n3. Maintain Resource\n4. Add Project\n5. Allocate Resource to Project\n6. Display Resource State\n7. Exit\nEnter your choice: ";
            std::cin >> choice;

            switch (choice) {
                case 1:
                    std::cout << "Enter Resource ID to add: ";
                    std::cin >> id;
                    std::cout << "Select Resource Type (1. Worker, 2. Equipment): ";
                    std::cin >> typeChoice;
                    type = (typeChoice == 1) ? "worker" : "equipment";
                    rm.addResource(id, type);
                    std::cout << "Resource " << id << " of type " << type << " added.\n";
                    rm.logTransaction("Resource " + id + " of type " + type + " added.");
                    break;
                case 2:
                    std::cout << "Enter Resource ID to use: ";
                    std::cin >> id;
                    rm.getResource(id)->setState(ResourceState::InUse);
                    std::cout << "Resource " << id << " is now in use.\n";
                    rm.logTransaction("Resource " + id + " is now in use.");
                    break;
                case 3:
                    std::cout << "Enter Resource ID to maintain: ";
                    std::cin >> id;
                    rm.maintainResource(id);
                    break;
                case 4:
                    std::cout << "Enter Project ID to add: ";
                    std::cin >> id;
                    std::cout << "Enter Project Name: ";
                    std::cin.ignore(); // Clear the newline character left in the buffer
                    std::getline(std::cin, name);
                    rm.addProject(id, name);
                    std::cout << "Project " << id << " named " << name << " added.\n";
                    rm.logTransaction("Project " + id + " named " + name + " added.");
                    break;
                case 5:
                    std::cout << "Enter Resource ID to allocate: ";
                    std::cin >> id;
                    std::cout << "Enter Project ID to allocate to: ";
                    std::cin >> name;
                    rm.allocateResourceToProject(id, rm.getProject(name));
                    std::cout << "Resource " << id << " allocated to project " << name << ".\n";
                    break;
                case 6:
                    std::cout << "Enter Resource ID to display state: ";
                    std::cin >> id;
                    rm.displayResourceState(id);
                    break;
                case 7:
                    std::cout << "Exiting...\n";
                    break;
                default:
                    std::cout << "Invalid choice. Please try again.\n";
            }
        } while (choice != 7);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
