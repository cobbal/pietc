#include "Program.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace pietc {
    
    using boost::shared_ptr;
    using std::list;
    using std::map;
    using std::pair;
    using std::string;
    
    void Program::explore(int x, int y, Codel * codel) {
        if (componentMap.count(std::make_pair(x, y)) > 0) {
            return;
        }
        componentMap[std::make_pair(x, y)] = codel;
        codel->size++;
        
        if (x + 1 < width && image.get(x, y) == image.get(x + 1, y)) {
            explore(x + 1, y, codel);
        }
        if (x - 1 > 0 && image.get(x, y) == image.get(x - 1, y)) {
            explore(x - 1, y, codel);
        }
        if (y + 1 < height && image.get(x, y) == image.get(x, y + 1)) {
            explore(x, y + 1, codel);
        }
        if (y - 1 > 0 && image.get(x, y) == image.get(x, y - 1)) {
            explore(x, y - 1, codel);
        }
    }
    
    bool Program::computeTransition(Codel * codel, int dp, int cc, Transition & tran) {
        if (codel->color == colors::white || codel->color == colors::black) {
            return false;
        }
        
        tran.from = codel;
        
        // Try to move
        for (int dpSpin = 0; dpSpin < 4; ++dpSpin) {
            for (int ccflip = 0; ccflip < 2; ++ccflip) {
                tran.obstacleTurnsDp = (dp + dpSpin) % 4;
                tran.obstacleFlipCc = (cc + ccflip) % 2;
                pair<int, int> newLoc = extremas[codel].directions[tran.obstacleTurnsDp][tran.obstacleFlipCc];
                
                const static int steps[4][2] = {
                    { 1, 0}, {0,  1},
                    {-1, 0}, {0, -1}
                };
                int xstep = steps[tran.obstacleTurnsDp][0];
                int ystep = steps[tran.obstacleTurnsDp][1];
                
                tran.opType = Transition::normal;
                
                while (true) {
                    newLoc.first += xstep;
                    newLoc.second += ystep;
                    
                    if (newLoc.first < 0 || newLoc.first >= width || 
                        newLoc.second < 0 || newLoc.second >= height) {
                        
                        break;
                    }
                    tran.to = componentMap[newLoc];
                    color_t newColor = tran.to->color;
                    if (newColor == colors::black) {
                        tran.to = NULL;
                        break;
                    }
                    
                    if (newColor != colors::white) {
                        // We have found our transition
                        return true;
                    }
                    tran.opType = Transition::noop;
                }
            } 
        }
        
        // This isn't reached unless program is done
        tran.opType = Transition::exit;
        tran.to = NULL;
        return true;
    }
    
    void Program::computeCodelTransitions(Codel * codel) {
        for (int dp = 0; dp < 4; dp++) {
            for (int cc = 0; cc < 2; cc++) {
                transitions.push_back(Transition());
                if (!computeTransition(codel, dp, cc, transitions.back())) {
                    transitions.pop_back();
                } else {
                    codel->transitions[dp][cc] = &transitions.back();
                }
            }
        }
    }
    
    int Program::indexOfCodel(Codel * codel) {
        int i = 0;
        BOOST_FOREACH(Codel & c, codels) {
            if (&c == codel) {
                return i;
            }
            i++;
        }
        return -1;
    }
    
    Program::Program(const char * filename) : image(filename), width(image.get_width()), height(image.get_height()) {
        std::cout << image << std::endl;
                
        // do a depth first search to discover connected components
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                pair<int, int> pos(x, y);
                if (componentMap.count(pos) == 0) {
                    color_t color = image.get(x, y);
                    codels.push_back(Codel(color));
                    explore(x, y, &codels.back());
                }
                std::cout << std::setw(3) << indexOfCodel(componentMap[pos]);
            }
            std::cout << std::endl;
        }
        
        typedef const pair<const pair<int, int>, Codel *> componentMapPair;
        BOOST_FOREACH(componentMapPair & pixel, componentMap) {
            for (int dp = 0; dp < 4; dp++) {
                for (int cc = 0; cc < 2; cc++) {
                    extremas[pixel.second].update(pixel.first, dp, cc);
                }
            }
        }
        
        BOOST_FOREACH(Codel & codel, codels) {
            computeCodelTransitions(&codel);
        }
        
        std::ofstream viz("out.dot");
        
        map<string, list<string> > edges;
        
        BOOST_FOREACH(Transition & tran, transitions) {
            std::stringstream edge;
            edge << "    \"" << indexOfCodel(tran.from) << " - " << colorName(tran.from->color) << "\" -> ";
            if (tran.to) {
                edge << "\"" << indexOfCodel(tran.to) << " - " << colorName(tran.to->color) << "\"";
                string dir;
                int dp, cc;
                for (dp = 0; dp < 4; dp++) {
                    for (cc = 0; cc < 2; cc++) {
                        if (tran.from->transitions[dp][cc] == &tran) {
                            goto outer_break; // cue velociraptors
                        }
                    }
                }
            outer_break:
                dir += "RDLU"[dp];
                dir += "lr"[cc];
                edges[edge.str()].push_back(dir);
            } else {
                edge << "EXIT";
                edges[edge.str()];
            }
        }
        
        viz << "digraph \"" << filename << "\" {" << std::endl;
        typedef const pair<string, list<string> > edgePair;
        BOOST_FOREACH(edgePair & e, edges) {
            viz << e.first << " [ label = \"";
            bool first = true;
            BOOST_FOREACH(const string & label, e.second) {
                viz << (first ? "" : ", ") << label;
                first = false;
            }
            viz << "\" ];" << std::endl;
        }
        viz << "}" << std::endl;
        viz.close();
    }
    
    Program::CodelExtrema::CodelExtrema() {
        for (int dp = 0; dp < 4; dp++) {
            for (int cc = 0; cc < 2; cc++) {
                directions[dp][cc] = std::make_pair(-1, -1);
            }
        }
    }
    
    void Program::CodelExtrema::update(pair<int, int> location, int dp, int cc) {
        pair<int, int> & current = directions[dp][cc];
        int xcmp = location.first - current.first;
        int ycmp = location.second - current.second;
        
        bool better = (current.first == -1);
        switch (dp) {
            case 0:
                better |= (xcmp > 0);
                better |= (xcmp == 0 && (cc == 0 ? (ycmp < 0) : (ycmp > 0)));
                break;
            case 1:
                better |= (ycmp > 0);
                better |= (ycmp == 0 && (cc == 0 ? (xcmp > 0) : (xcmp < 0)));
                break;
            case 2:
                better |= (xcmp < 0);
                better |= (xcmp == 0 && (cc == 0 ? (ycmp > 0) : (ycmp < 0)));
                break;
            case 3:
                better |= (ycmp < 0);
                better |= (ycmp == 0 && (cc == 0 ? (xcmp < 0) : (xcmp > 0)));
                break;
            default:
                assert(false);
        }
        if (better) {
            directions[dp][cc] = location;
        }
    }
    
} // namespace pietc
