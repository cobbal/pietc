namespace pietc {

class Codel;

class Transition {
public:
    enum OperationType {
        normal,
        noop,
        exit
    };

    Codel* from;
    Codel* to;
    OperationType opType;

    int obstacleTurnsDp;
    bool obstacleFlipCc;
};

} // namespace pietc
