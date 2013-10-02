#ifndef VERSIONCONTROLPLUGIN_H
#define VERSIONCONTROLPLUGIN_H

#include "Connection.h"
#include "Command.h"
#include "Status.h"
#include "VersionedAsset.h"
#include "Changes.h"
#include <string>
#include <map>
#include <set>

#define ENUM_FLAGS(T) \
inline T  operator  |(const T s, const T e) { return (T)((unsigned)s | e); } \
inline T &operator |=(T      &s, const T e) { return s = s | e; }            \
inline T  operator  &(const T s, const T e) { return (T)((unsigned)s & e); } \
inline T &operator &=(T      &s, const T e) { return s = s & e; }            \
inline T  operator  ^(const T s, const T e) { return (T)((unsigned)s ^ e); } \
inline T &operator ^=(T      &s, const T e) { return s = s ^ e; }            \
inline T  operator  ~(const T s)            { return (T)~(unsigned)s; }

enum VersionControlUnityVersion { kUnity42 = 1, kUnity43 = 2 };

/*
 * VersionControl Plugin configuration field.
 */
class VersionControlPluginCfgField {
public:
    enum FieldFlags
    {
        kNoneField = 0,
        kRequiredField = 1,
        kPasswordField = 2
    };

    VersionControlPluginCfgField()
    {
        m_Name = "";
        m_Label = "";
        m_Description = "";
        m_DefaultValue = "";
        m_Flags = kNoneField;
        m_Value = "";
    }
    
    VersionControlPluginCfgField(const std::string& name, const std::string& label, const std::string& description, const std::string& defaultValue, FieldFlags flags)
    {
        m_Name = name;
        m_Label = label;
        m_Description = description;
        m_DefaultValue = defaultValue;
        m_Flags = flags;
        m_Value = m_DefaultValue;
    }
    
    VersionControlPluginCfgField(const VersionControlPluginCfgField& other)
    {
        m_Name = other.GetName();
        m_Label = other.GetLabel();
        m_Description = other.GetDescription();
        m_DefaultValue = other.GetDefaultValue();
        m_Flags = (FieldFlags)other.GetFlags();
        m_Value = other.GetValue();
    }
    
	const std::string& GetName() const { return m_Name; }
    const std::string& GetLabel() const { return m_Label; }
	const std::string& GetDescription() const { return m_Description; }
    const std::string& GetDefaultValue() const { return m_DefaultValue; }
    const std::string& GetValue() const { return m_Value; }
	void SetValue(const std::string& newValue) { m_Value = newValue; }
    int GetFlags() const { return m_Flags; }

    VersionControlPluginCfgField& operator= (const VersionControlPluginCfgField& rhs)
    {
        m_Name = rhs.GetName();
        m_Label = rhs.GetLabel();
        m_Description = rhs.GetDescription();
        m_DefaultValue = rhs.GetDefaultValue();
        m_Flags = (FieldFlags)rhs.GetFlags();
        m_Value = rhs.GetValue();
        
        return (*this);
    }

private:
	std::string m_Name;
	std::string m_Label;
	std::string m_Description;
	std::string m_DefaultValue;
	std::string m_Value;
	FieldFlags m_Flags;
};

ENUM_FLAGS(VersionControlPluginCfgField::FieldFlags);

Connection& operator<<(Connection& p, const VersionControlPluginCfgField& field);

/*
 * VersionControl Plugin abstract class
 */
class VersionControlPlugin
{
public:
    typedef std::map<std::string, std::string> VersionControlPluginMapOfArguments;
    typedef std::set<int> VersionControlPluginVersions;
    typedef std::vector<VersionControlPluginCfgField> VersionControlPluginCfgFields;
    typedef std::map<State, std::string> VersionControlPluginOverlays;
    
    enum TraitsFlags {
        kRequireNetwork = 1 << 0,
        kEnablesCheckout = 1 << 1,
        kEnablesLocking = 1 << 2,
        kEnablesRevertUnchanged = 1 << 3,
        kEnablesVersioningFolders = 1 << 4,
        kEnablesChangelists = 1 << 5,
        kEnablesGetLatestOnChangeSetSubset = 1 << 6,
        kEnablesConflictHandlingByPlugin = 1 << 7
    };
    
    enum CommandsFlags {
        kAdd = 1 << 0,
		kChangeDescription = 1 << 1,
        kChangeMove = 1 << 2,
		kChanges = 1 << 3,
        kChangeStatus = 1 << 4,
        kCheckout = 1 << 5,
		kDeleteChanges = 1 << 6,
		kDelete = 1 << 7,
        kDownload = 1 << 8,
		kGetLatest = 1 << 9,
        kIncomingChangeAssets = 1 << 10,
        kIncoming = 1 << 11,
		kLock = 1 << 12,
        kMove = 1 << 13,
        kResolve = 1 << 14,
		kRevertChanges = 1 << 15,
        kRevert = 1 << 16,
        kStatus = 1 << 17,
		kSubmit = 1 << 18,
        kUnlock = 1 << 19,
        kAll = (1 << 20) - 1
    };
    
    // Main loop
    int Run();

protected:
    
    // Constructors
    VersionControlPlugin();
    VersionControlPlugin(int argc, char** argv);
    VersionControlPlugin(const char* args);
    
    // Online/Offline
    inline bool IsOnline() const { return m_IsOnline; }
    inline void SetOnline() { m_IsOnline = true; }
    inline void SetOffline() { m_IsOnline = false; }
    void NotifyOffline(const std::string& reason);
    void NotifyOnline();
    
    // Get plugin connection with Unity.
    Connection& GetConnection() { return (*m_Connection); }

    // ProjectPath getter/setter
    inline void SetProjectPath(const std::string& path) { m_ProjectPath = path; }
    inline const std::string& GetProjectPath() const { return m_ProjectPath; }
    
    // VCStatus
	const VCSStatus& GetStatus() const;
	void ClearStatus();
    VCSStatus& StatusAdd(VCSStatusItem& item);
    
    /*
     * Get the log file name.
     * Returns:
     *  - string that reperesent the log file name.
     */
    virtual const char* GetLogFileName() = 0;
    
    /*
     * Get the supported Unity versions.
     * Returns:
     *  - set of supported Unity versions.
     */
    virtual const VersionControlPluginVersions& GetSupportedVersions() = 0;
    
    /*
     * Get the supported plugin traits.
     * Returns:
     *  - The supported plugin traits.
     */
    virtual const TraitsFlags GetSupportedTraitFlags() = 0;
    
    /*
     * Get the plugin configuration fields.
     * Returns:
     *  - list of plugin configuration fields.
     */
    virtual VersionControlPluginCfgFields& GetConfigFields() = 0;
    
    /*
     * Get the commands that should be enabled when plugin is online.
     * Returns:
     *  - the commands to enable when plugin is online.
     */
    virtual CommandsFlags GetOnlineUICommands() = 0;
    
    /*
     * Get the plugin overlays (with icons if not default).
     * Returns:
     *  - the plugin overlays.
     */
    virtual const VersionControlPluginOverlays& GetOverlays() { return s_emptyOverlays; };
    
    //
    virtual int Connect() = 0;
    
    //
    virtual void Disconnect() = 0;
    
    //
    virtual bool IsConnected() { return true; }
    
    //
    virtual int Login() { return 0; }
    
    /*
     * Add assets to VC.
     * Parameters: 
     *  - assetList: IN/OUT list of versioned asset.
     * Returns:
     *  - True if operation succeeded, false otherwise (VCStatus contains errors).
     *
     * On input, assetList contains assests to be added.
     * On output, assetList contains assests that have been added with appropriate status.
     */
    virtual bool AddAssets(VersionedAssetList& assetList) = 0;
    
    /*
     * Checkout assets in VC.
     * Parameters:
     *  - assetList: IN/OUT list of versioned asset.
     * Returns:
     *  - True if operation succeeded, false otherwise (VCStatus contains errors).
     *
     * On input, assetList contains assests to be checked out.
     * On output, assetList contains assests that have been checked out with appropriate status.
     */
    virtual bool CheckoutAssets(VersionedAssetList& assetList) = 0;
    
    /*
     * Download assets from VC.
     * Parameters:
     *  - targetDir: IN target directory.
     *  - changes: IN list of changes.
     *  - assetList: IN/OUT list of versioned asset.
     * Returns:
     *  - True if operation succeeded, false otherwise (VCStatus contains errors).
     *
     * On input, assetList contains assests to download.
     * On output, assetList contains assests that have been downloaded from VC with appropriate status.
     */
    virtual bool DownloadAssets(const std::string& targetDir, const ChangelistRevisions& changes, VersionedAssetList& assetList) = 0;
    
    /*
     * Get assets from VC.
     * Parameters:
     *  - assetList: IN/OUT list of versioned asset.
     * Returns:
     *  - True if operation succeeded, false otherwise (VCStatus contains errors).
     *
     * On input, assetList contains assests to get.
     * On output, assetList contains assests that have been get from VC with appropriate status.
     */
    virtual bool GetAssets(VersionedAssetList& assetList) = 0;
    
    /*
     * Revert assets from VC.
     * Parameters:
     *  - assetList: IN/OUT list of versioned asset.
     * Returns:
     *  - True if operation succeeded, false otherwise (VCStatus contains errors).
     *
     * On input, assetList contains assests to be reverted.
     * On output, assetList contains assests that have been reverted with appropriate status.
     */
    virtual bool RevertAssets(VersionedAssetList& assetList) = 0;
    
    /*
     * Resolve assets from VC.
     * Parameters:
     *  - assetList: IN/OUT list of versioned asset.
     * Returns:
     *  - True if operation succeeded, false otherwise (VCStatus contains errors).
     *
     * On input, assetList contains assests to be resolved.
     * On output, assetList contains assests that have been resolved with appropriate status.
     */
    virtual bool ResolveAssets(VersionedAssetList& assetList) = 0;
    
    /*
     * Remove assets from VC.
     * Parameters:
     *  - assetList: IN/OUT list of versioned asset.
     * Returns:
     *  - True if operation succeeded, false otherwise (VCStatus contains errors).
     *
     * On input, assetList contains assests to be removed.
     * On output, assetList contains assests that have been removed with appropriate status.
     */
    virtual bool RemoveAssets(VersionedAssetList& assetList) = 0;
    
    //
    virtual bool MoveAssets(const VersionedAssetList& fromAssetList, VersionedAssetList& toAssetList) = 0;
    
    /*
     * Lock assets in VC.
     * Parameters:
     *  - assetList: IN/OUT list of versioned asset.
     * Returns:
     *  - True if operation succeeded, false otherwise (VCStatus contains errors).
     *
     * On input, assetList contains assests to be locked.
     * On output, assetList contains assests that have been locked with appropriate status.
     */
    virtual bool LockAssets(VersionedAssetList& assetList) = 0;
    
    /*
     * Unlock assets in VC.
     * Parameters:
     *  - assetList: IN/OUT list of versioned asset.
     * Returns:
     *  - True if operation succeeded, false otherwise (VCStatus contains errors).
     *
     * On input, assetList contains assests to be unlocked.
     * On output, assetList contains assests that have been unlocked with appropriate status.
     */
    virtual bool UnlockAssets(VersionedAssetList& assetList) = 0;
    
    //
    virtual bool ChangeOrMoveAssets(const ChangelistRevision& revision, VersionedAssetList& assetList) = 0;
    
    //
    virtual bool SubmitAssets(const Changelist& changeList, VersionedAssetList& assetList) = 0;
    
    /*
     * Get assets status from VC.
     * Parameters:
     *  - assetList: IN/OUT list of versioned asset.
     *  - recursive: IN boolean, true if status is made recursively
     * Returns:
     *  - True if operation succeeded, false otherwise (VCStatus contains errors).
     *
     * On input, assetList contains assests for which status is asked.
     * On output, assetList contains assests with appropriate status.
     */
    virtual bool GetAssetsStatus(VersionedAssetList& assetList, bool recursive = false) = 0;
    
    //
    virtual bool GetAssetsChangeStatus(const ChangelistRevision& revision, VersionedAssetList& assetList) = 0;
    
    //
    virtual bool GetIncomingAssetsChangeStatus(const ChangelistRevision& revision, VersionedAssetList& assetList) = 0;
    
    //
    virtual bool GetAssetsChanges(Changes& changes) = 0;
    
    //
    virtual bool GetAssetsIncomingChanges(Changes& changes) = 0;
    
    //
    virtual bool UpdateRevision(const ChangelistRevision& revision, std::string& description) = 0;
    
    //
    virtual bool DeleteRevision(const ChangelistRevision& revision) = 0;
    
    //
    virtual bool RevertChanges(const ChangelistRevision& revision, VersionedAssetList& assetList) = 0;

private:
    void InitializeArguments(int argc, char** argv);
    
    bool PreHandleCommand();
    void PostHandleCommand(bool wasOnline);

    bool HandleAdd();
    bool HandleChangeDescription();
    bool HandleChangeMove();
    bool HandleChanges();
    bool HandleChangeStatus();
    bool HandleCheckout();
    bool HandleConfig(const CommandArgs& args);
    bool HandleConfigTraits();
    bool HandleCustomCommand(const CommandArgs& args);
    bool HandleDelete();
    bool HandleDeleteChanges();
    bool HandleDownload();
    bool HandleExit();
    bool HandleFileMode();
    bool HandleGetlatest();
    bool HandleIncoming();
    bool HandleIncomingChangeAssets();
    bool HandleLock();
    bool HandleLogin();
    bool HandleMove(const CommandArgs& args);
    bool HandleQueryConfigParameters();
    bool HandleResolve();
    bool HandleRevert();
    bool HandleRevertChanges();
    bool HandleSetConfigParameters(const CommandArgs& args);
    bool HandleStatus(const CommandArgs& args);
    bool HandleSubmit(const CommandArgs& args);
    bool HandleUnlock();

    bool HandleVersionedAssetsCommand(UnityCommand cmd);
    bool Dispatch(UnityCommand command, const CommandArgs& args);
    
    int SelectVersion(const VersionControlPluginVersions& unitySupportedVersions);

	std::string m_ProjectPath;
    VCSStatus m_Status;
    VersionControlPluginMapOfArguments m_arguments;
    
    bool m_IsOnline;
    Connection* m_Connection;
    
    static VersionControlPluginOverlays s_emptyOverlays;
};

ENUM_FLAGS(VersionControlPlugin::TraitsFlags);

ENUM_FLAGS(VersionControlPlugin::CommandsFlags);

#endif // VERSIONCONTROLPLUGIN_H