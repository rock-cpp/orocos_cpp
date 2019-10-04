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


bool orocos_cpp::PkgConfigRegistry::loadPackages(const std::vector<std::string>& packageNames, const std::vector<std::string>& searchPaths)
{
    bool st = true;
    for(const std::string& pname : packageNames ){
        st &= findAndLoadPackage(pname, searchPaths);
    }
    return st;
}

orocos_cpp::PkgConfigRegistryPtr orocos_cpp::PkgConfigRegistry::initialize(const std::vector<std::string>& packageNames, bool loadAllPackages)
{
    if(__pkgcfgreg){
        LOG_WARN_S << "PkgConfigRegistry::initialize was already called earlier!";
    }
    __pkgcfgreg = PkgConfigRegistryPtr(new PkgConfigRegistry(packageNames, loadAllPackages));

    return __pkgcfgreg;
}

bool orocos_cpp::PkgConfigRegistry::getDeployment(const std::string &name, orocos_cpp::PkgConfig &pkg, bool searchPackageIfNotLoaded)
{
    std::map<std::string, PkgConfig>::iterator it = deployments.find(name);
    if(it == deployments.end()){
        if(!searchPackageIfNotLoaded){
            return false;
        }
        LOG_DEBUG_S << "Deployment Package " << name << " was requested but is not present in PkgConfigregistry. Trying to find it.";
        bool st = findAndLoadPackage(name);
        if(st){
            return getDeployment(name, pkg, false);
        }else{
            return false;
        }
    }
    pkg = it->second;
    return true;
}

bool orocos_cpp::PkgConfigRegistry::getTypekit(const std::string &name, orocos_cpp::TypekitPkgConfig &pkg, bool searchPackageIfNotLoaded)
{
    std::map<std::string, TypekitPkgConfig>::iterator it = typekits.find(name);
    if(it == typekits.end()){
        if(!searchPackageIfNotLoaded){
            return false;
        }
        LOG_DEBUG_S << "Typekit Package " << name << " was requested but is not present in PkgConfigregistry. Trying to find it.";
        bool st = findAndLoadPackage(name);
        if(st){
            return getTypekit(name, pkg, false);
        }else{
            return false;
        }
    }
    pkg = it->second;
    return true;
}

bool orocos_cpp::PkgConfigRegistry::getOrogen(const std::string &name, orocos_cpp::OrogenPkgConfig &pkg, bool searchPackageIfNotLoaded)
{
    std::map<std::string, OrogenPkgConfig>::iterator it = orogen.find(name);
    if(it == orogen.end()){
        if(!searchPackageIfNotLoaded){
            return false;
        }
        LOG_DEBUG_S << "Orogen Package " << name << " was requested but is not present in PkgConfigregistry. Trying to find it.";
        bool st = findAndLoadPackage(name);
        if(st){
            return getOrogen(name, pkg, false);
        }else{
            return false;
        }
    }
    pkg = it->second;
    return true;
}

bool orocos_cpp::PkgConfigRegistry::getOrocosRTT(orocos_cpp::PkgConfig &pkg, bool searchPackageIfNotLoaded)
{
    if(!orocosRTTPkg.isLoaded()){
        if(!searchPackageIfNotLoaded){
            return false;
        }
        LOG_DEBUG_S << "The Orocos-RTT Package was requested but is not present in PkgConfigregistry. Trying to find it.";
        bool st = findAndLoadPackage("rtt");
        if(st){
            return getOrocosRTT(pkg, false);
        }else{
            return false;
        }
    }else{
        pkg = orocosRTTPkg;
        return true;
    }
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
        LOG_WARN_S << "PkgConfigRegistry::get was called before initializing it. This was okay in previous versions, but is deprecated now! Call PkgConfigRegistry::initialize once before using PkgConfigRegistry::get.";
        return PkgConfigRegistry::initialize({}, false);
        //throw std::runtime_error("PkgConfigRegistry::get was called before initializing it. Did you forget to call OrocosCpp::initialize?");
    }
    return __pkgcfgreg;
}

orocos_cpp::PkgConfigRegistry::PkgConfigRegistry(const std::vector<std::string> &packageNames, bool loadAllPackages)
{
    std::vector<std::string> searchPaths = PkgConfigHelper::getSearchPathsFromEnvVar();
    if(loadAllPackages){
        this->loadAllPackages(searchPaths);
    }else{
        loadPackages(packageNames, searchPaths);
    }
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


bool load_pkg(const fs::path& pkg_path, orocos_cpp::PkgConfig& pkg)
{
    if(!fs::exists(pkg_path)){
        LOG_WARN_S << "File " << pkg_path << " not found.";
    }else{
        bool st = pkg.load(pkg_path.string());
        if(st){
            LOG_INFO_S << "Loading PkgConfig " << pkg_path << "\t[OK]";
        }else{
            LOG_ERROR_S << "Loading PkgConfig " << pkg_path << "\t[FAILED]";
        }
        return st;
    }
    return false;
}


bool orocos_cpp::PkgConfigRegistry::loadOrogenPkg(const fs::path &filepath)
{
    //Extract name of orogen package, target and init transports
    std::string filename = filepath.filename().string();
    std::string package_name = filename.substr(std::string("orogen-project-").size(), filename.size()-std::string("orogen-project-").size()-std::string(".pc").size());
    const fs::path pkg_search_path = filepath.parent_path();
    std::string target = std::getenv("OROCOS_TARGET");
    std::vector<std::string> known_transports = {"corba","mqueue","typelib"};

    //Load PkgConfig file of OrogenProject
    PkgConfig pkg_proj;
    bool st = pkg_proj.load(filepath.string());
    if(!st){
        LOG_ERROR_S << "Error loading PkgConfig file of OroGen Project-package "<<package_name<<" from "<<filepath.string();
        return st;
    }
    OrogenPkgConfig opkg;
    opkg.project = pkg_proj;

    //Load Tasks package, if it is defined
    PkgConfig pkg;
    fs::path pkg_path = pkg_search_path / (package_name + "-tasks-"+target+".pc");
    st = load_pkg(pkg_path, pkg);
    opkg.tasks = pkg;
    //Load Proxies package, if it is defined
    pkg_path = pkg_search_path / (package_name + "-proxies" + ".pc");
    st = load_pkg(pkg_path, pkg);
    opkg.proxies = pkg;

    //store orogen package
    orogen[package_name] = opkg;

    //Load typekit-PkgConfig, if it is defined
    pkg_path = pkg_search_path / (package_name + "-typekit-"+target+".pc");
    st = load_pkg(pkg_path, pkg);
    if(st){
        TypekitPkgConfig tpkg;
        tpkg.typekit = pkg;

        //Load transports-PkgConfig, if it is defined
        for(const std::string& t : known_transports){
            pkg_path = pkg_search_path / (package_name + "-transport-" + t + "-"  +target + ".pc");
            st = load_pkg(pkg_path, pkg);
            if(st){
                tpkg.transports[t] = pkg;
            }
        }
        typekits[package_name] = tpkg;
    }
    return true;
}

bool orocos_cpp::PkgConfigRegistry::loadDeploymentPkg(const fs::path &filepath)
{
    //Extract name of orogen package, target and init transports
    std::string filename = filepath.filename().string();
    std::string package_name = filename.substr(std::string("orogen-").size(), filename.size()-std::string("orogen-").size()-std::string(".pc").size());

    //Load deployment
    PkgConfig pkg;
    bool st = load_pkg(filepath, pkg);
    deployments[package_name] = pkg;
    return st;
}

bool orocos_cpp::PkgConfigRegistry::findAndLoadPackage(const std::string& pname, std::vector<std::string> searchPaths)
{
    if(searchPaths.size() == 0){
        searchPaths = PkgConfigHelper::getSearchPathsFromEnvVar();
    }
    fs::path fpath;
    bool found = false;

    LOG_DEBUG_S << "Searching for package " << pname;
    for(const fs::path& path : searchPaths){
        if(!fs::is_directory(path)){
            LOG_WARN_S << "Skipping directory " << path << " since it is not a valid directory";
            continue;
        }

        //Check for rtt package
        if(pname == "rtt" || pname == "orocos-rtt"){
            std::string target = std::getenv("OROCOS_TARGET");
            fpath = path / ("orocos-rtt-"+target+".pc");
            if(fs::exists(fpath)){
                LOG_DEBUG_S << "PkgConfig for RTT Package was found in file " << fpath;
                PkgConfig pkg;
                bool st = pkg.load(fpath.string());
                if(st){
                    LOG_INFO_S << "PkgConfig " << fpath << "  for the RTT Package was sucessfully loaded";
                    orocosRTTPkg = pkg;
                    found = true;
                }else{
                    LOG_ERROR_S << "Error loading Orocos-RTT PkgConfig file from " << fpath;
                }

                //load transports for rtt
                TypekitPkgConfig tpkg;
                std::vector<std::string> known_transports = {"corba","mqueue","typelib"};
                for(const std::string& t : known_transports){
                    fpath = path / ("orocos-rtt-"+t+"-"+target+".pc");
                    if(pkg.load(fpath.string())){
                        LOG_INFO_S << "Loaded transport " << t << " for RTT package from " << fpath;
                        tpkg.transports[t] = pkg;
                    }else{
                        LOG_INFO_S << "Could not load transport " << t << " for RTT package from " << fpath;
                    }
                }
                typekits["rtt"] = tpkg;
                typekits["orocos-rtt"] = tpkg;
            }
        }

        //Check for oroGen package
        fpath = path / ("orogen-project-"+pname+".pc");
        if(fs::exists(fpath)){
            LOG_DEBUG_S << "PkgConfig for Orogen Package with name " << pname << " found in file " << fpath;
            bool st = loadOrogenPkg(fpath);
            if(st){
                LOG_INFO_S << "PkgConfig " << fpath << "  for Orogen Package with name " << pname << " was sucessfully loaded";
                found = true;
            }else{
                LOG_ERROR_S << "Error loading PkgConfig file " << fpath;
            }
        }

        //Check for Deployment package
        fpath = path / ("orogen-"+pname+".pc");
        if(fs::exists(fpath)){
            LOG_DEBUG_S << "PkgConfig for Orogen Deployment Package with name " << pname << " found in file " << fpath;
            bool st = loadDeploymentPkg(fpath);
            if(st){
                LOG_INFO_S << "PkgConfig " << fpath << "  for Orogen Deployment Package with name " << pname << " was sucessfully loaded";
                found = true;
            }
            else{
                LOG_ERROR_S << "Error loading PkgConfig file of orogen deployment package " << fpath;
            }
        }
    }
    return found;
}

void orocos_cpp::PkgConfigRegistry::loadAllPackages(const std::vector<std::string> &searchPaths)
{
    LOG_INFO_S << "Loading all packages defined in search path";
    for(const std::string& path : searchPaths){
        LOG_DEBUG_S << "Scanning " << path;
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

