#include "data.h"

Paper papers[] = {
    {
        "Estimation of Warfarin Dosage using XGBoost-based Pharmacogenomic ML & XAI",
        "Warfarin dosing remains clinically challenging due to its narrow therapeutic "
        "window and high interpatient variability. This paper presents a specialized "
        "XGBoost-based pharmacogenomic model integrating CYP2C9 and VKORC1 polymorphisms "
        "with clinical variables to predict stable warfarin doses. Explainability is "
        "evaluated using XAI techniques to surface the most influential features.",
        "https://doi.org/10.1109/icmlc66258.2025.11280120"
    },
    {
        "CYBRANA: YARA/YAML-driven AI Firewall",
        "CYBRANA is an AI-powered firewall framework leveraging YARA rules and YAML "
        "configuration for dynamic cyber attack detection and log analysis. Integrates "
        "MITRE ATT&CK framework mappings for automated threat classification. Achieves "
        "91.7% attack classification accuracy with sub-millisecond detection latency.",
        "https://doi.org/10.1109/CVMI61877.2024.10782249"
    },
    {
        "FLARE: Federated Learning And Resilient Encryption for Firewalls",
        "Traditional firewalls rely on static rule-based mechanisms easily bypassed by "
        "evolving malware. FLARE proposes a federated learning-based firewall that "
        "dynamically analyzes past network traffic patterns across distributed nodes, "
        "combined with resilient encryption to preserve data privacy.",
        "https://doi.org/10.1109/punecon63413.2024.10895282"
    },
    {
        "SHADOW: Systematic Heuristic Analysis and Detection of Observations on the Web",
        "SHADOW proposes a data ingestion pipeline integrating surface, deep, and dark "
        "web sources via advanced scraping. Data is stored in a private cloud and "
        "Proof-of-Authority blockchain for immutability. An NLP pipeline ranks results "
        "to surface actionable threat intelligence and detect data exposure in real time.",
        "https://doi.org/10.1109/icamac62387.2024.10828750"
    },
    {
        "Enhancing Resilience of Privacy-Preserving ML using Adversarial Techniques",
        "This paper proposes a framework integrating adversarial techniques with "
        "differential privacy and homomorphic encryption to enhance ML resilience. "
        "Hardens models against evasion and membership inference attacks while "
        "preserving data confidentiality across distributed learning settings.",
        "https://doi.org/10.1109/icdscnc62492.2024.10939481"
    },
};
size_t num_papers = sizeof(papers) / sizeof(papers[0]);

Link about_links[] = {
    {"resume",    "industry recruiters",  "https://aadi.zip/resume"},
    {"cv",        "academic evaluators",  "https://aadi.zip/cv"},
    {"portfolio", "freelance work",       "https://aadi.zip/portfolio"},
    {"linkedin",  "professional network", "https://linkedin.com/in/aaditya-rengarajan"},
    {"email",     "direct contact",       "aaditya.r@ieee.org"},
};
size_t num_links = sizeof(about_links) / sizeof(about_links[0]);
