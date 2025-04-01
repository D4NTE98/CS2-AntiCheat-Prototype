class BanManager {
private:
    std::map<uint64_t, std::vector<Violation>> violations;
    
public:
    void AddViolation(uint64_t steam_id, Violation v) {
        violations[steam_id].push_back(v);
        if (ShouldBan(violations[steam_id])) {
            ScheduleDelayedBan(steam_id);
        }
    }
    
    void ScheduleDelayedBan(uint64_t steam_id) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(12h, 72h);
        
        std::this_thread::sleep_for(dist(gen));
        IssueBan(steam_id);
    }
};
