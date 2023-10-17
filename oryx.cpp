#include <iostream>
#include <vector>
#include <string>

using namespace std;

class ORYX {
public:

    //Инициализирует три линейных обратных сдвиговых регистра (LFSRA, LFSRB, LFSRK) размером 32 бита, заполняя их нулями
    //Инициализирует вектор SBox размером 256 элементов, который будет использоваться как таблица замены
    ORYX() {
        LFSRA.resize(32, 0);
        LFSRB.resize(32, 0);
        LFSRK.resize(32, 0);
        SBox.resize(256);
        generateSBox();
    }

    //Заполняет SBox идентичными значениями, т.е. каждый элемент SBox содержит свой индекс
    void generateSBox() {
        for (int i = 0; i < 256; ++i) {
            SBox[i] = i;
        }
    }

    //Реализация функции обратной связи для каждого из трех регистров
    //Возвращает результат вычисления суммы по модулю 2 некоторых битов регистров
    int feedbackFunctionLFSRK() {
        return (
            LFSRK[31] + LFSRK[27] + LFSRK[26] + LFSRK[25] +
            LFSRK[24] + LFSRK[23] + LFSRK[22] + LFSRK[17] +
            LFSRK[13] + LFSRK[11] + LFSRK[10] + LFSRK[9] +
            LFSRK[8] + LFSRK[7] + LFSRK[2] + LFSRK[0]
            ) % 2;
    }

    //Реализация функции обратной связи для каждого из трех регистров
    //Возвращает результат вычисления суммы по модулю 2 некоторых битов регистров
    int feedbackFunctionLFSRA() {
        return (
            LFSRA[31] + LFSRA[26] + LFSRA[23] + LFSRA[22] +
            LFSRA[16] + LFSRA[12] + LFSRA[11] + LFSRA[10] +
            LFSRA[8] + LFSRA[7] + LFSRA[5] + LFSRA[4] +
            LFSRA[2] + LFSRA[0]
            ) % 2;
    }

    //Реализация функции обратной связи для каждого из трех регистров
    //Возвращает результат вычисления суммы по модулю 2 некоторых битов регистров
    int feedbackFunctionLFSRB() {
        return (
            LFSRB[31] + LFSRB[30] + LFSRB[20] + LFSRB[19] +
            LFSRB[15] + LFSRB[14] + LFSRB[5] + LFSRB[2] +
            LFSRB[0]
            ) % 2;
    }

    //Осуществляет циклический сдвиг элементов регистра на одну позицию вправо
    void shiftRegisters(vector<int>& registerState) {
        for (int i = registerState.size() - 1; i > 0; --i) {
            registerState[i] = registerState[i - 1];
        }
    }

    //Обновляет состояние регистров, вызывая соответствующие функции обратной связи и выполняя циклический сдвиг
    void updateRegisters() {
        int feedbackLFSRK = feedbackFunctionLFSRK();
        int feedbackLFSRA = feedbackFunctionLFSRA();
        int feedbackLFSRB = feedbackFunctionLFSRB();

        shiftRegisters(LFSRK);
        shiftRegisters(LFSRA);
        shiftRegisters(LFSRB);

        LFSRK[0] = feedbackLFSRA ^ feedbackLFSRB;
        LFSRA[0] = feedbackLFSRK;
        LFSRB[0] = feedbackLFSRK ^ feedbackLFSRA;
    }

    //Формирует байт ключевого потока
    //Использует 8 бит из регистра LFSRA и LFSRB, применяет SBox к комбинированному вектору, затем выполняет операцию сложения по модулю 256
    int generateKeystreamByte() {
        int high8K = 0;
        vector<int> high8A(LFSRA.begin() + 24, LFSRA.end());
        vector<int> high8B(LFSRB.begin() + 24, LFSRB.end());

        vector<int> combinedVector;
        combinedVector.reserve(high8A.size() + high8B.size());
        combinedVector.insert(combinedVector.end(), high8A.begin(), high8A.end());
        combinedVector.insert(combinedVector.end(), high8B.begin(), high8B.end());

        for (int& value : combinedVector) {
            value = SBox[value];
        }

        int keystreamByte = (high8K + sumVector(combinedVector)) % 256;

        return keystreamByte;
    }

    //Вычисляется сумма значений вектора
    int sumVector(const vector<int>& vec) {
        int sum = 0;
        for (int value : vec) {
            sum += value;
        }
        return sum;
    }

    //Шифрует входной текст, применяя операцию XOR между каждым символом входного текста и соответствующим байтом ключевого потока.
    //Для каждого символа текста вызывается метод updateRegisters() и generateKeystreamByte().
    vector<int> encrypt(const string& plaintext) {
        vector<int> ciphertext;
        for (char ch : plaintext) {
            updateRegisters();
            int keystreamByte = generateKeystreamByte();
            ciphertext.push_back(ch ^ keystreamByte);
        }
        return ciphertext;
    }

    //Дешифратор
    vector<int> decrypt(const vector<int>& ciphertext) {
        vector<int> plaintext;
        for (int byte : ciphertext) {
            updateRegisters();
            int keystreamByte = generateKeystreamByte();
            plaintext.push_back(byte ^ keystreamByte);
        }
        return plaintext;
    }

private:
    vector<int> LFSRA;
    vector<int> LFSRB;
    vector<int> LFSRK;
    vector<int> SBox;
};


int main() {
    ORYX oryxCipher;

    string plaintext;
    cout << "Enter text: ";
    getline(cin, plaintext);

    vector<int> ciphertext = oryxCipher.encrypt(plaintext);

    cout << "Cipher text: ";
    for (int byte : ciphertext) {
        cout << byte << " ";
    }
    cout << endl;

    //Дешифратор
    vector<int> decryptedText = oryxCipher.decrypt(ciphertext);

    cout << "Decrypted text: ";
    for (int byte : decryptedText) {
        cout << char(byte);
    }
    cout << endl;

    return 0;
}