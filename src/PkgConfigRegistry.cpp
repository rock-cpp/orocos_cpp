#include "PkgConfigRegistry.hpp"
#include "PkgConfigHelper.hpp"
#include <regex>
#include <boost/filesystem.hpp>
#include <base-logging/Logging.hpp>

orocos_cpp::PkgConfigRegistryPtr orocos_cpp::__pkgcfgreg(nullptr);

template<typename T>
void extract_keys(const std::map<std::string,T>& m, std::vector<std::string>& res){
    for(const auto& kv : m) {
        res.push_back(kv.first);
    }
}

orocos_cpp::PkgConfig::PkgConfig() :
    sourceFile("")
{

}

bool orocos_cpp::PkgConfig::getVariable(const std::string &fieldName, std::string& value)
{
    std::map<std::string, std::string>::iterator it = variables.find(fieldName);
    if(it == variables.end())
        return false;

    value = it->second;
    return true;
}

bool orocos_cpp::PkgConfig::getProperty(const std::string &fieldName, std::string& value)
{
    std::map<std::string, std::string>::iterator it = properties.find(fieldName);
    if(it == properties.end())
        return false;

    value = it->second;
    return true;
}

bool orocos_cpp::PkgConfig::load(const std::string &filepath)
{
    bool st = PkgConfigHelper::parsePkgConfig(filepath, variables, properties);
    bool got_name = getProperty("Name", name);
    sourceFile = filepath;
    return st;
}

bool orocos_cpp::PkgConfig::isLoaded()
{
    return !this->sourceFile.empty();
}


bool orocos_cpp::PkgConfigRegistry::initialize()
{
    LOG_INFO_S << "Scanning for PKGConfig files for orogen project, typekits, transports, tasks and deployments";
    std::vector<std::string> searchPaths = PkgConfigHelper::getSearchPathsFromEnvVar();
    scan(searchPaths);
    LOG_INFO_S << "Scanning for PKG config files done.";
    return true;
}

bool orocos_cpp::PkgConfigRegistry::getDeployment(const std::string &name, orocos_cpp::PkgConfig &pkg)
{
    std::map<std::string, PkgConfig>::iterator it = deployments.find(name);
    if(it == deployments.end()){
        return false;
    }
    pkg = it->second;
    return true;
}

bool orocos_cpp::PkgConfigRegistry::getTypekit(const std::string &name, orocos_cpp::TypekitPkgConfig &pkg)
{
    std::map<std::string, TypekitPkgConfig>::iterator it = typekits.find(name);
    if(it == typekits.end()){
        return false;
    }
    pkg = it->second;
    return true;
}

bool orocos_cpp::PkgConfigRegistry::getOrogen(const std::string &name, orocos_cpp::OrogenPkgConfig &pkg)
{
    std::map<std::string, OrogenPkgConfig>::iterator it = orogen.find(name);
    if(it == orogen.end()){
        return false;
    }
    pkg = it->second;
    return true;
}

bool orocos_cpp::PkgConfigRegistry::getOrocosRTT(orocos_cpp::PkgConfig &pkg)
{
    pkg = orocosRTTPkg;
    return orocosRTTPkg.isLoaded();
}

std::vector<std::string> orocos_cpp::PkgConfigRegistry::getRegisteredDeploymentNames()
{
    std::vector<std::string> ret;
    extract_keys(deployments, ret);
    return ret;
}

std::vector<std::string> orocos_cpp::PkgConfigRegistry::getRegisteredTypekitNames()
{
    std::vector<std::string> ret;
    extract_keys(typekits, ret);
    return ret;
}

std::vector<std::string> orocos_cpp::PkgConfigRegistry::getRegisteredOrogenNames()
{
    std::vector<std::string> ret;
    extract_keys(orogen, ret);
    return ret;
}

orocos_cpp::PkgConfigRegistryPtr orocos_cpp::PkgConfigRegistry::get()
{
    if(!__pkgcfgreg){
        __pkgcfgreg = PkgConfigRegistryPtr(new PkgConfigRegistry());
        __pkgcfgreg->initialize();
    }
    return __pkgcfgreg;
}

bool orocos_cpp::PkgConfigRegistry::isTransportPkg(const std::string &filename, std::string &typekitName, std::string &transportName, std::string& arch)
{
    std::regex r(R"(([\w\d]+)-transport-([\w]+)-([\w]+).pc)");
    std::smatch match;
    if(std::regex_match(filename, match, r)){
        LOG_DEBUG_S << filename << " is Transport:";
        typekitName = match[1];
        transportName = match[2];
        arch = match[3];
        LOG_DEBUG_S << "  typekit: "<<typekitName<<"\n  trasnport: "<<transportName<<"\n  arch: "<<arch;
        return true;
    }
    return false;
}

bool orocos_cpp::PkgConfigRegistry::isOrogenTasksPkg(const std::string &filename, std::string &orogenProjectName, std::string& arch)
{
    std::regex r(R"(([\w\d]+)-tasks-([\w\d]+).pc)");
    std::smatch match;
    if(std::regex_match(filename, match, r)){
        LOG_DEBUG_S << filename << " is OrogenTasks:";
        orogenProjectName = match[1];
        arch = match[2];
        LOG_DEBUG_S << "  orogen: "<<orogenProjectName<<"\n  arch: "<<arch;
        return true;
    }
    return false;
}

bool orocos_cpp::PkgConfigRegistry::isOrogenProjectPkg(const std::string &filename, std::string &orogenProjectName)
{
    std::regex r(R"(orogen-project-([\w\d]+).pc)");
    std::smatch match;
    if(std::regex_match(filename, match, r)){
        LOG_DEBUG_S << filename << " is OrogenProject:";
        orogenProjectName = match[1];
        LOG_DEBUG_S << "  orogen: "<<orogenProjectName;
        return true;
    }
    return false;
}

bool orocos_cpp::PkgConfigRegistry::isTypekitPkg(const std::string &filename, std::string &typekitName, std::string& arch)
{
    std::regex r(R"(([\w\d]+)-typekit-([\w\d]+).pc)");
    std::smatch match;
    if(std::regex_match(filename, match, r)){
        LOG_DEBUG_S << filename << " is Typekit:";
        typekitName = match[1];
        arch = match[2];
        LOG_DEBUG_S << "  typekit: "<<typekitName;
        LOG_DEBUG_S << "  arch: "<<arch;
        return true;
    }
    return false;
}

bool orocos_cpp::PkgConfigRegistry::isProxiesPkg(const std::string &filename, std::string &orogenProjectName)
{
    std::regex r(R"(([\w\d]+)-proxies.pc)");
    std::smatch match;
    if(std::regex_match(filename, match, r)){
        LOG_DEBUG_S << filename << " is Proxies:";
        orogenProjectName = match[1];
        LOG_DEBUG_S << "  orogen: "<<orogenProjectName;
        return true;
    }
    return false;
}

bool orocos_cpp::PkgConfigRegistry::isDeploymentPkg(const std::string &filename, std::string &deploymentName)
{
    std::regex r(R"(orogen-([\w\d]+).pc)");
    std::smatch match;
    if(std::regex_match(filename, match, r)){
        LOG_DEBUG_S << filename << " is Deployment:";
        deploymentName = match[1];
        LOG_DEBUG_S << "  depoyment: "<<deploymentName;
        return true;
    }
    return false;
}

bool orocos_cpp::PkgConfigRegistry::isOrocosRTTPkg(const std::string &filename, std::string &arch)
{
    std::regex r(R"(orocos-rtt-([\w\d]+).pc)");
    std::smatch match;
    if(std::regex_match(filename, match, r)){
        LOG_DEBUG_S << filename << " is Orocos RTT:";
        arch = match[1];
        LOG_DEBUG_S << "  arch: "<<arch;
        return true;
    }
    return false;
}

bool orocos_cpp::PkgConfigRegistry::addFile(const std::string &filepath)
{
    std::string typekitName, orogenProjectName, deploymentName, arch, transportName;
    PkgConfig pkg;
    boost::filesystem::path p(filepath);
    std::string filename = p.filename().string();

    //In Rock different special kinds of libraries can be identified by patterns in their file
    //names. Following the type of a library is identifyied by the name of the corresponding
    //PkgConfig file.

    //Is Deployment?
    if(isDeploymentPkg(filename, deploymentName)){
        bool st = pkg.load(filepath);
        std::map<std::string, PkgConfig>::iterator it = deployments.find(deploymentName);
        if(it != deployments.end()){
            LOG_WARN_S << "Ignoring PKGConfig file "<<filepath<<", because it describes a deployment with name " << deploymentName << ", but there was already a PKGConfig file for the same deployment added with the file " << it->second.sourceFile << ".";
            return false;
        }
        deployments[deploymentName] = pkg;
        return st;
    }

    //Is Proxy?
    else if(isProxiesPkg(filename, orogenProjectName)){
        bool st = pkg.load(filepath);
        std::map<std::string, OrogenPkgConfig>::iterator it = orogen.find(orogenProjectName);

        if(it == orogen.end()){
            orogen[orogenProjectName] = OrogenPkgConfig();
            it = orogen.find(orogenProjectName);
        }

        if(it->second.proxies.isLoaded()){
            LOG_WARN_S << "Ignoring PKGConfig file "<<filepath<<", because it describes task proxies for the orogen project " << orogenProjectName << ", but they were already be imported from the PKGConfig file " << it->second.proxies.sourceFile << ".";
            return false;
        }
        it->second.proxies = pkg;
        return st;
    }

    //Is OrogenProject
    else if(isOrogenProjectPkg(filename, orogenProjectName)){
        bool st = pkg.load(filepath);
        std::map<std::string, OrogenPkgConfig>::iterator it = orogen.find(orogenProjectName);

        if(it == orogen.end()){
            orogen[orogenProjectName] = OrogenPkgConfig();
            it = orogen.find(orogenProjectName);
        }

        if(it->second.project.isLoaded()){
            LOG_WARN_S << "Ignoring PKGConfig file "<<filepath<<", because it describes the orogen project " << orogenProjectName << ", but it was already described by the PKGConfig file " << it->second.project.sourceFile << ".";
            return false;
        }
        it->second.project = pkg;
        return st;
    }

    //Is OrogenTasks
    else if(isOrogenTasksPkg(filename, orogenProjectName, arch)){
        bool st = pkg.load(filepath);
        std::map<std::string, OrogenPkgConfig>::iterator it = orogen.find(orogenProjectName);

        if(it == orogen.end()){
            orogen[orogenProjectName] = OrogenPkgConfig();
            it = orogen.find(orogenProjectName);
        }

        if(it->second.tasks.isLoaded()){
            LOG_WARN_S << "Ignoring PKGConfig file "<<filepath<<", because it describes the tasks for the orogen project " << orogenProjectName << ", but the tasks were already described by the PKGConfig file " << it->second.tasks.sourceFile << ".";
            return false;
        }
        it->second.tasks = pkg;
        return st;
    }

    //Is Transports
    else if(isTransportPkg(filename, typekitName, transportName, arch)){
        bool st = pkg.load(filepath);
        std::map<std::string, TypekitPkgConfig>::iterator it = typekits.find(typekitName);

        if(it == typekits.end()){
            typekits[typekitName] = TypekitPkgConfig();
            it = typekits.find(typekitName);
        }

        std::map<std::string, PkgConfig>::iterator transportit = it->second.transports.find(transportName);
        if(transportit != it->second.transports.end()){
            LOG_WARN_S << "Ignoring PKGConfig file "<<filepath<<", because it describes the transport " << transportName << " for typekit " << typekitName << ", but the transport was already described by the PKGConfig file " << transportit->second.sourceFile << ".";
            return false;
        }
        it->second.transports[transportName] = pkg;
        return st;
    }

    //Is Typekit
    else if(isTypekitPkg(filename, typekitName, arch)){
        bool st = pkg.load(filepath);
        std::map<std::string, TypekitPkgConfig>::iterator it = typekits.find(typekitName);
        if(it == typekits.end()){
            typekits[typekitName] = TypekitPkgConfig();
            it = typekits.find(typekitName);
        }

        if(it->second.typekit.isLoaded()){
            LOG_WARN_S << "Ignoring PKGConfig file "<<filepath<<", because it describes the typekit " << typekitName << ", but the typekit was already described by the PKGConfig file " << it->second.typekit.sourceFile << ".";
            return false;
        }
        it->second.typekit = pkg;
        return st;
    }

    //IS OrocosRTT
    //RTT follows a different convention. Kind of library is determined by folder they are installed in.
    else if(isOrocosRTTPkg(filename, arch)){
        if(orocosRTTPkg.isLoaded()){
            LOG_WARN_S << "Ignoring PkgConfig file " << filepath << ". It describes the package orocos-rtt, but that was already described by the PkgConfig file "<<orocosRTTPkg.sourceFile;
            return false;
        }

        bool st = pkg.load(filepath);
        orocosRTTPkg = pkg;
        return st;
    }
    else{
        //Do nothing.. we don't load pkgconfig files for arbitrary libraries, since we'll not need them.
        return false;
    }
}

void orocos_cpp::PkgConfigRegistry::scan(const std::vector<std::string> &searchPaths)
{
    namespace fs = boost::filesystem;

    for(const std::string& path : searchPaths){
        if(!fs::is_directory(path)){
            LOG_WARN_S << "Skipping directory " << path << " since it is not a valid directory";
            continue;
        }
        for (fs::directory_iterator itr(path); itr!=fs::directory_iterator(); ++itr)
        {
            if( fs::is_regular_file(*itr) ){
                bool st = addFile(itr->path().string());
                if(st){
                    LOG_INFO_S << "Loaded PkgConfig file " << itr->path().string() << "\t" << "[OK]";
                }else{
                    LOG_INFO_S << "Loaded PkgConfig file " << itr->path().string() << "\t" << "[IGNORED]";
                }
            }
        }
    }
}

