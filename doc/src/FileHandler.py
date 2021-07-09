class FileHandler:        
    def __init__(self, fileName=None):
        self.contents = []
        if(fileName != None):
            with open(fileName, errors='ignore') as f:
                self.contents = f.readlines()
            # remove whitespace characters like `\n` at the end of each line
            self.contents = [x.strip() for x in self.contents]
    
    def at(self, lineNumber):
        return self.contents[lineNumber]
        
    def getContent(self):
        result = ""
        for c in self.contents:
            result += c
            result += "\n"
        return result
        
    def reprint(self, fileName):
        f = open(fileName, "w")
        for c in self.contents:
            f.write(c)
            f.write("\n")
        f.close()
    
    def replaceLine(self, involvedLine, toPut, instances = 0):
        indices = self.__findIndices(involvedLine, instances)
        for i in indices:
            self.contents[i] = toPut

    def removeLine(self, toRemove, instances = 0):
        indices = self.__findIndices(toRemove, instances)
        for c in range(0, len(indices), 1):
            self.contents.remove(toRemove)
    
    def addBefore(self, involvedLine, toPut, instances = 0):
        indices = self.__findIndices(involvedLine, instances)
        added = 0
        for i in indices:
            self.contents.insert(i + added, toPut)
            added = added + 1

    def addAfter(self, involvedLine, toPut, instances = 0):
        indices = self.__findIndices(involvedLine, instances)
        added = 0
        for i in indices:
            self.contents.insert(i + added + 1, toPut)
            added = added + 1
    
    def pushBack(self, toPut):
        self.contents.append(toPut)
    
    def pushFront(self, toPut):
        self.contents.insert(0, toPut)
        
    def __findIndices(self, involvedLine, instances):
        if(instances == 0):
            instances = len(self.contents)
        indices = []
        k = 0
        K = 0
        for l in self.contents:
            if(l == involvedLine):
                indices.append(k)
                K = K + 1
                if(K == instances):
                    break
            k = k + 1
        return indices
