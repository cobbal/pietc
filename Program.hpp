#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include "Codel.hpp"
#include "Transition.hpp"
#include "ProgramImage.hpp"
#include <list>
#include <map>

namespace pietc {
    
    class Program {
    public:
        Program(const char * filename);
    private:
        struct CodelExtrema {
            CodelExtrema();
            void update(std::pair<int, int> location, int dp, int cc);
            
            std::pair<int, int> directions[4][2];
        };
                
        ProgramImage image;
        std::list<Codel> codels;
        std::list<Transition> transitions;
        
        void explore(int x, int y, Codel * codel);
        void computeCodelTransitions(Codel * codel);
        bool computeTransition(Codel * codel, int dp, int cc, Transition & tran);
        int indexOfCodel(Codel * codel);
        
        const unsigned int width;
        const unsigned int height;
        std::map<Codel *, CodelExtrema> extremas;
        std::map<std::pair<int, int>, Codel *> componentMap;
    };
    
} // namespace pietc

#endif // __PROGRAM_H__
