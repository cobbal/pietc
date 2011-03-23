namespace pietc {

class ColorBlock;

class Transition {
public:
    enum OperationType {
        normal,
        noop,
        exit
    };

    ColorBlock* from;
    ColorBlock* to;
    OperationType opType;

    int obstacleTurnsDp;
    bool obstacleFlipCc;
};

} // namespace pietc
