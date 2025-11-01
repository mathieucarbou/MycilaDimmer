#include <cmath>
#include <iostream>
#include <iomanip>

// Calculer le courant et la puissance de chaque harmonique

struct HarmonicData {
    int order;           // Ordre de l'harmonique (1, 3, 5, 7, ...)
    float percent;       // Pourcentage par rapport au fondamental
    float current_rms;   // Courant RMS en ampères
    float power;         // Puissance "apparente" de l'harmonique en watts
    float frequency;     // Fréquence en Hz
};

void calculateHarmonicPower(float voltage, float total_current_rms, float resistance, 
                            const float* harmonics, size_t n, HarmonicData* output) {
    
    // Calculer le courant RMS du fondamental
    // I_total² = I1² + I3² + I5² + I7² + ...
    // Donc: I1² = I_total² / (1 + (H3/100)² + (H5/100)² + ...)
    
    float sum_squares = 1.0f; // H1 = 100% = 1.0
    for (size_t i = 1; i < n; i++) {
        float h_normalized = harmonics[i] / 100.0f;
        sum_squares += h_normalized * h_normalized;
    }
    
    float i1_rms = total_current_rms / sqrtf(sum_squares);
    
    std::cout << "Courant total RMS: " << total_current_rms << " A" << std::endl;
    std::cout << "Courant fondamental (50Hz): " << i1_rms << " A" << std::endl;
    std::cout << std::endl;
    
    // Calculer pour chaque harmonique
    for (size_t i = 0; i < n; i++) {
        int order = 2 * i + 1;
        float h_percent = harmonics[i];
        float i_n_rms = i1_rms * (h_percent / 100.0f);
        
        // Pour une charge résistive, la puissance de chaque harmonique
        // P_n = I_n² × R (mais seul H1 est utile!)
        float p_n = i_n_rms * i_n_rms * resistance;
        
        output[i].order = order;
        output[i].percent = h_percent;
        output[i].current_rms = i_n_rms;
        output[i].power = p_n;
        output[i].frequency = 50.0f * order; // 50Hz, 150Hz, 250Hz, ...
    }
}

void printHarmonicTable(const HarmonicData* data, size_t n) {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "┌──────┬──────────┬───────────┬───────────┬───────────┐" << std::endl;
    std::cout << "│  Hn  │  %H1     │ I_n (A)   │ P_n (W)   │ Freq (Hz) │" << std::endl;
    std::cout << "├──────┼──────────┼───────────┼───────────┼───────────┤" << std::endl;
    
    float total_harmonic_power = 0.0f;
    
    for (size_t i = 0; i < n; i++) {
        std::cout << "│ H" << std::setw(2) << data[i].order << "  │ ";
        std::cout << std::setw(7) << data[i].percent << "% │ ";
        std::cout << std::setw(8) << data[i].current_rms << "  │ ";
        std::cout << std::setw(8) << data[i].power << "  │ ";
        std::cout << std::setw(8) << data[i].frequency << "  │" << std::endl;
        
        if (i > 0) // Exclure H1 du total des harmoniques
            total_harmonic_power += data[i].power;
    }
    
    std::cout << "└──────┴──────────┴───────────┴───────────┴───────────┘" << std::endl;
    std::cout << std::endl;
    
    std::cout << "⚠️  IMPORTANT:" << std::endl;
    std::cout << "  - Puissance active UTILE = " << data[0].power << " W (H1 uniquement!)" << std::endl;
    std::cout << "  - Puissance harmoniques = " << total_harmonic_power << " W" << std::endl;
    std::cout << "  - Les harmoniques NE FONT QUE chauffer les fils et perturber!" << std::endl;
    std::cout << "  - Seul H1 (50Hz) produit un travail utile sur une charge résistive" << std::endl;
}

int main() {
    std::cout << "=== Analyse Courant et Puissance par Harmonique ===" << std::endl;
    std::cout << std::endl;
    
    // Exemple avec vos données réelles
    float voltage = 236.5f;        // Tension secteur en V
    float total_current = 1.951f;  // Courant total RMS mesuré en A
    float power_active = 259.2f;   // Puissance active mesurée en W
    float resistance = power_active / (total_current * total_current); // R = P / I²
    
    std::cout << "Données mesurées:" << std::endl;
    std::cout << "  Tension: " << voltage << " V" << std::endl;
    std::cout << "  Courant total: " << total_current << " A" << std::endl;
    std::cout << "  Puissance active: " << power_active << " W" << std::endl;
    std::cout << "  Résistance équivalente: " << resistance << " Ω" << std::endl;
    std::cout << std::endl;
    
    // Harmoniques à 113° (duty ~37%)
    float harmonics[] = {100.0f, 19.93f, 9.99f, 16.31f, 11.15f, 1.85f, 5.57f, 7.79f, 5.93f, 1.73f, 2.91f};
    size_t n = 11;
    
    HarmonicData data[11];
    calculateHarmonicPower(voltage, total_current, resistance, harmonics, n, data);
    
    printHarmonicTable(data, n);
    
    std::cout << std::endl;
    std::cout << "=== Calcul d'énergie ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Si le dimmer fonctionne pendant 1 heure:" << std::endl;
    std::cout << "  Énergie utile (H1): " << (data[0].power / 1000.0f) << " kWh" << std::endl;
    std::cout << "  Énergie gaspillée (harmoniques): " << 0.0f << " kWh (!)" << std::endl;
    std::cout << std::endl;
    std::cout << "Explication:" << std::endl;
    std::cout << "  Sur une résistance pure, TOUTES les harmoniques produisent" << std::endl;
    std::cout << "  de la chaleur. Donc toute l'énergie est 'utile' pour chauffer!" << std::endl;
    std::cout << "  P_totale = I_total² × R = " << (total_current * total_current * resistance) << " W" << std::endl;
    std::cout << std::endl;
    std::cout << "  Les harmoniques ne 'gaspillent' pas d'énergie sur une résistance," << std::endl;
    std::cout << "  mais elles créent:" << std::endl;
    std::cout << "    - Des pertes dans les câbles (échauffement)" << std::endl;
    std::cout << "    - Des perturbations EMI/RFI" << std::endl;
    std::cout << "    - Une dégradation du facteur de puissance" << std::endl;
    
    return 0;
}
