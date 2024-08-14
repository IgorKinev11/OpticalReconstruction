#include "Material.h"

#include <random>

void Material::deepCopy(const std::map<coefs, SurfaceOpticProperty*>& sourceMap)
{

    for (auto& coef : sourceMap)
    {
        switch (coef.first)
        {
        case coefs::kd:
            optical_coef_map[coef.first] = new Kd(coef.second->getColor(), coef.second->getCoeff());
            break;
        //case coefs::ks:
        //    optical_coef_map[coef.first] = new Ks(coef.second->getColor(), coef.second->getCoeff());
        //    break;
        //case coefs::ktd:
        //    optical_coef_map[coef.first] = new Ktd(coef.second->getColor(), coef.second->getCoeff());
        //    break;
        //case coefs::kts:
        //    optical_coef_map[coef.first] = new Kts(coef.second->getColor(), coef.second->getCoeff());
        //    break;
        //case coefs::brdf:
        //    optical_coef_map[coef.first] = new BRDF(color, coef.second->getCoeff(), 1);
        //    break;
        }
    }
}

Material::Material(const Material& other)
{
    deepCopy(other.optical_coef_map);
}

Material& Material::operator=(const Material& other)
{
    if (this != &other)
    {
        for (auto& pair : optical_coef_map)
        {
            delete pair.second;
        }
        optical_coef_map.clear();

        deepCopy(other.optical_coef_map);
    }
    return *this;
}

Material::~Material()
{
    for (auto& coef : optical_coef_map)
    {
        delete coef.second;
    }
}

Material::Material(Color& color, std::map<coefs, float> optical_coef_float_map)
{

    for (const auto& coef : optical_coef_float_map)
    {
        switch (coef.first)
        {
            case coefs::kd:
                optical_coef_map[coef.first] = new Kd(color, coef.second);
                break;
            //case coefs::ks:
            //    optical_coef_map[coef.first] = new Ks(color, coef.second);
            //    break;
            //case coefs::ktd:
            //    optical_coef_map[coef.first] = new Ktd(color, coef.second);
            //    break;
            //case coefs::kts:
            //    optical_coef_map[coef.first] = new Kts(color, coef.second);
            //    break;
            //case coefs::brdf:
            //    optical_coef_map[coef.first] = new BRDF(color, coef.second, 1);
            //    break;
        }
    }
}

Color Material::CalculateLuminance(const Color& E, const Vec3f& U, const Vec3f& V, const Vec3f& N) const
{
    Color luminance;

    for (auto& coef : optical_coef_map)
    {
        if (coef.second)
        {
            luminance = luminance + coef.second->CalculateLuminance(E, U, V, N);
        }
    }

    return luminance;
}

Ray Material::TransformRay(const Ray& ray, const Vec3f& N, const Vec3f& intersectionPoint) const
{
    auto event = chooseEvent(ray);
    if (event == nullptr)
    {
        Ray transformed;
        transformed.trash.lastEvent = TransformRayEvent::e_KILL;
        return transformed;
    }

    return event->TransformRay(ray, N, intersectionPoint);
}

SurfaceOpticProperty* Material::chooseEvent(const Ray& ray) const
{
    static std::mt19937                          gen;
    static std::uniform_real_distribution<float> dist(0, 1);

    for (auto& coef : optical_coef_map)
    {
        if (coef.second)
        {
            // Вероятность выбора события
            float prob = coef.second->getColor().dot(ray.color) * coef.second->getCoeff();

            // Выбор события
            float xi   = dist(gen);
            if (xi < prob)
                return coef.second;
        }
    }

    return nullptr;
}

void Material::NormalizeCoefs(SurfaceOpticProperty* prior_coef)
{
    int   count = optical_coef_map.size();
    float sum   = 1.0;
    float delta;

    if (prior_coef->getCoeff() < 0) // ???
    {
        prior_coef->setCoeff(0);
    }

    if (prior_coef)
    {
        prior_coef->setCoeff(std::min(prior_coef->getCoeff(), 1.0f));
        sum -= prior_coef->getCoeff();
        count -= 1;
    }

    for (auto& coef : optical_coef_map)
    {
        if (coef.second != prior_coef)
        {
            sum -= coef.second->getCoeff();
        }
    }

    if (sum < 0)
    {
        delta = -sum / count;
        for (auto& coef : optical_coef_map)
        {
            if (coef.second != prior_coef)
            {
                coef.second->setCoeff(std::max(coef.second->getCoeff() - delta, 0.0f));
            }
        }
    }
}
